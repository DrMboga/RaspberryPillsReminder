#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include <time.h>
#include <pthread.h>
#include "leds-service.h"
#include "file-access.h"
#include "leds.h"
#include "messaging.h"
#include "utils.h"

// Blink function reference
void *blinkFunc();
// Synchronisation critical section
pthread_mutex_t blinkMutex = PTHREAD_MUTEX_INITIALIZER;
// Array which contains a PED pins which should blink
int LedsToBlink[LEDS_COUNT];
// Amount of blinking LEDs
int BlinkingLedsCount = 0;

// Native Gpio messages listener function reference
void *NativeGpioListenerFunc();

int main()
{
    // Reading LEDs state from csv file
    struct LedStateRow* ledsState = ReadLedsState();

    // Reading LED Pins mapping
    struct PinMapping* ledPins = GetLedsMapping();

    // Setting up LEDs state in a board
    if(SetUpLedsState(ledsState, ledPins) < 0)
    {
        perror("Error setting up LEDs state");
        return -1;
    }

    // Start a new thread which listens the Native GPIO control events
    pthread_t threadForNativeGpio;
    int threadCreated;
    if( (threadCreated=pthread_create( &threadForNativeGpio, NULL, &NativeGpioListenerFunc, NULL)) )
    {
        printf("Native GPIO thread creation failed: %d\n", threadCreated);
    }

    // Initialize Leds Message queue
    int messageQueueId = InitializeMessageQueue(LED_CONTROL_MESSAGE_TYPE);
    
    if(messageQueueId >= 0)
    {
        printf("Successfully got messageQueueId: %d\n", messageQueueId);
        while(1) 
        {
            printf("Waiting for incoming message...\n");
            struct LedsControlMsessage received;
            // If no LEDs is blinking, current thread is sleeping while waiting new message from the queue
            if (msgrcv(messageQueueId, &received, sizeof(received.LedAction) + sizeof(received.LedNumber) + sizeof(received.LedColor), LED_CONTROL_MESSAGE_TYPE, 0)< 0)
            {
                PrintMessagingError(errno);
                if(errno == EINVAL || errno == EIDRM)
                {
                    printf("Re-initializing the queue\n");
                    messageQueueId = InitializeMessageQueue(LED_CONTROL_MESSAGE_TYPE);
                    printf("New messageQueueId: %d\n", messageQueueId);
                    if(messageQueueId == -1)
                    {
                        return 1;
                    }
                }
            }
            logReceivedMessageInfo(received.LedNumber, received.LedAction, received.LedColor);

            applyLedAction(received.LedNumber, received.LedColor, received.LedAction, ledPins);
            RewriteLedsState(received.LedNumber, received.LedColor, received.LedAction, ledsState);
        }
        msgctl(messageQueueId, IPC_RMID, NULL);
    }

    free(ledsState);
    free(ledPins);
    return 0;
}

/*
* Sets up LEDs state in the board
*/
int SetUpLedsState(struct LedStateRow* ledsState, struct PinMapping* ledPins)
{
    int color = -1;
    int action = -1;
    for (int i = 0; i < LEDS_COUNT; i++)
    {
        switch(ledsState[i].state)
        {
            case LEDS_OFF:
                action = LED_OFF;
                break;
            case LED_GREEN_ON:
                action = LED_ON;
                color = GREEN_LED;
                break;
            case LED_GREEN_BLINK:
                action = LED_BLINK;
                color = GREEN_LED;
                break;
            case LED_RED_ON:
                action = LED_ON;
                color = RED_LED;
                break;
            case LED_RED_BLINK:
                action = LED_BLINK;
                color = RED_LED;
                break;
        }
        applyLedAction(ledsState[i].ledNumber, color, action, ledPins);

    }

    return 0;
}

/*
* Finds LED pin and applies an action to it
*/
void applyLedAction(int ledNumber, int color, int action, struct PinMapping* ledPins)
{
    for (int j = 0; j < LEDS_COUNT; j++)
    {
        if(ledPins[j].ledNumber == ledNumber)
        {
            InitLed(ledPins[j].redPin);
            InitLed(ledPins[j].greenPin);
            stopBlink(ledPins[j].redPin, ledPins[j].greenPin);
            switch(action)
            {
                case LED_OFF:
                    TurnBiColorLedOff(ledPins[j].redPin);
                    TurnBiColorLedOff(ledPins[j].greenPin);
                    break;
                case LED_ON:
                    switch(color)
                    {
                        case GREEN_LED:
                            TurnBiColorLedOff(ledPins[j].redPin);
                            TurnBiColorLedOn(ledPins[j].greenPin);
                            break;
                        case RED_LED:
                            TurnBiColorLedOff(ledPins[j].greenPin);
                            TurnBiColorLedOn(ledPins[j].redPin);
                            break;
                    }
                    break;
                case LED_BLINK:
                    switch(color)
                    {
                        case GREEN_LED:
                            TurnBiColorLedOff(ledPins[j].redPin);
                            startBlink(ledPins[j].greenPin);
                            break;
                        case RED_LED:
                            TurnBiColorLedOff(ledPins[j].greenPin);
                            startBlink(ledPins[j].redPin);
                            break;
                    }
                    break;
            }
            break;
        }
    }
}

/*
* Reads LEDs state from csv file
*/
struct LedStateRow* ReadLedsState()
{
    int ledNumber;
    int state;
    char time[6];
    int rowResult;
    int ledIndex = 0;
    struct LedStateRow* leds = malloc(LEDS_COUNT * sizeof(struct LedStateRow)); //leds[LEDS_COUNT];

    FILE* ledsStateFilePointer = OpenFileForRead(LEDS_STATE_FILE_NAME);

    if(ledsStateFilePointer == NULL)
    {
        // File doesn't exist yet
        for (int i = 0; i < LEDS_COUNT; i++)
        {
            leds[i].ledNumber = LEDS_COUNT - 1 - i;
            leds[i].state = LEDS_OFF;
            strcpy(leds[i].time, EMPTY_TIME);
        }

        return leds;
    }

    while ((rowResult = fscanf(ledsStateFilePointer, "%d\t%d\t%s\n", &ledNumber, &state, time)) != EOF)
    {
        if(rowResult >= 3)
        {
            leds[ledIndex].ledNumber = ledNumber;
            leds[ledIndex].state = state;
            strcpy(leds[ledIndex].time, time);
            ledIndex++;
        }
        if(ledIndex >= LEDS_COUNT)
        {
            break;
        }
    }

    CloseFile(ledsStateFilePointer);

    return leds;
}

/*
* Writes a new LEDs state into csv file
*/
int RewriteLedsState(int ledNumber, int color, int action, struct LedStateRow* ledsState)
{
    for (int i = 0; i < LEDS_COUNT; i++)
    {
        if(ledsState[i].ledNumber == ledNumber)
        {
            // curent time
            char currentTime[6];
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            strftime(currentTime, 6, "%R", timeinfo);
            switch(action)
            {
                case LED_OFF:
                    ledsState[i].state = LEDS_OFF;
                    strcpy(ledsState[i].time, EMPTY_TIME);
                    break;
                case LED_ON:
                    switch(color)
                    {
                        case GREEN_LED:
                            ledsState[i].state = LED_GREEN_ON;
                            strcpy(ledsState[i].time, currentTime);
                            break;
                        case RED_LED:
                            ledsState[i].state = LED_RED_ON;
                            strcpy(ledsState[i].time, currentTime);
                            break;
                    }
                    break;
                case LED_BLINK:
                    switch(color)
                    {
                        case GREEN_LED:
                            ledsState[i].state = LED_GREEN_BLINK;
                            strcpy(ledsState[i].time, currentTime);
                            break;
                        case RED_LED:
                            ledsState[i].state = LED_RED_BLINK;
                            strcpy(ledsState[i].time, currentTime);
                            break;
                    }
                    break;
            }
            break;
        }
    }
    
    FILE* ledsStateFilePointer = OpenFileForRewrite(LEDS_STATE_FILE_NAME);
    for (int i = 0; i < LEDS_COUNT; i++)
    {
        if(fprintf(ledsStateFilePointer, "%d\t%d\t%s\n", ledsState[i].ledNumber, ledsState[i].state, ledsState[i].time) < 0)
        {
            printf("Write operaion failed");
            return -1;
        }
    }
    return CloseFile(ledsStateFilePointer);
}

/*
* Prints to the console message with information
*/
void logReceivedMessageInfo(int ledNumber, int action, int color)
{
    char stringAction[6];
    char stringColor[6];
    switch (action)
    {
        case LED_BLINK:
            strcpy(stringAction, "BLINK");
            break;
        case LED_OFF:
            strcpy(stringAction, "OFF");
            break;
        case LED_ON:
            strcpy(stringAction, "ON");
            break;
    }  
    switch (color)
    {
    case GREEN_LED:
        strcpy(stringColor, "GREEN");
        break;
    case RED_LED:
        strcpy(stringColor, "RED");
        break;
    }

    printf("Received message with action %s, led number %d and color %s\n", stringAction, ledNumber, stringColor);
}

/*
* Adds pin number to the state array and start new thread for blinking
*/
void startBlink(int ledPin)
{
    // Check, if state array doesn't have elements yet, then we need to start a new thread
    int needToSatrtNewThread = 0;
    if(BlinkingLedsCount == 0)
    {
        needToSatrtNewThread = 1;
    }
    // Entering critical section
    pthread_mutex_lock( &blinkMutex );
    int needToAddPinToArray = 1;
    // Check if current led pin is already in the state array
    for (int i = 0; i < BlinkingLedsCount; i++)
    {
        if(LedsToBlink[i] == ledPin)
        {
            needToAddPinToArray = 0;
        }
    }
    // If current led is not in the state array, add it to the array
    if(needToAddPinToArray == 1)
    {
        LedsToBlink[BlinkingLedsCount] = ledPin;
        BlinkingLedsCount++;
    }

    // Exit critical section
    pthread_mutex_unlock( &blinkMutex );

    if(needToSatrtNewThread == 1)
    {
        // If no leds are blinking yet, starting a new thread with blink procedure
        pthread_t threadForBlinking;
        int threadCreated;
        if( (threadCreated=pthread_create( &threadForBlinking, NULL, &blinkFunc, NULL)) )
        {
            printf("Thread creation failed: %d\n", threadCreated);
        }
    }
}

/*
* Remove pin number from the state array
*/
void stopBlink(int redPin, int greenPin)
{
    int indexToDelete = -1;
    for (int i = 0; i < BlinkingLedsCount; i++)
    {
        if(LedsToBlink[i] == redPin || LedsToBlink[i] == greenPin)
        {
            indexToDelete = i;
        }
    }

    if(indexToDelete < 0)
    {
        return;
    }

    // If indexToDelete is not the last in the array, so we have to pop it and shift the rest elements to the left
    // Entering critical section
    pthread_mutex_lock( &blinkMutex );
    for (int i = indexToDelete; i < BlinkingLedsCount; i++)
    {
        // Shift next item to the left
        if(i < BlinkingLedsCount - 1)
        {
            printf("Removing pin number %d from blinking LEDs array index %d and replacing it by %d\n", LedsToBlink[i], i, LedsToBlink[i+1]);
            LedsToBlink[i] = LedsToBlink[i+1];
        }
        else
        {
            // Last element
            printf("Removing pin number %d from blinking LEDs array index %d\n", LedsToBlink[i], i);
            LedsToBlink[i] = -1;
        }
    }
    BlinkingLedsCount--;
    // Exit critical section
    pthread_mutex_unlock( &blinkMutex );
}

void *blinkFunc()
{
    printf("New thread for blink is started\n");
    int blinkingLedsCountAtTheMoment = 0;
    // Entering critical section
    pthread_mutex_lock( &blinkMutex );
    blinkingLedsCountAtTheMoment = BlinkingLedsCount;
    // Exit critical section
    pthread_mutex_unlock( &blinkMutex );

    int ledsState = 1;

    printf("Number of LEDs to blink %d\n", blinkingLedsCountAtTheMoment);
    while (blinkingLedsCountAtTheMoment > 0)
    {
        // Entering critical section
        pthread_mutex_lock( &blinkMutex );
        for (int i = 0; i < BlinkingLedsCount; i++)
        {
            if(ledsState == 1)
            {
                TurnBiColorLedOn(LedsToBlink[i]);
            }
            else
            {
                TurnBiColorLedOff(LedsToBlink[i]);
            }
        }
        blinkingLedsCountAtTheMoment = BlinkingLedsCount;
        // Exit critical section
        pthread_mutex_unlock( &blinkMutex );

        if(ledsState == 1)
        {
            ledsState = 0;
        }
        else
        {
            ledsState = 1;
        }

        sleepMilliseconds(500);
    }

    printf("Blinking thread exits.\n");
}

void *NativeGpioListenerFunc()
{
    int gpioMessageQueueId = InitializeMessageQueue(NATIVE_GPIO_MESSAGE_TYPE);
    if(gpioMessageQueueId >= 0)
    {
        printf("Successfully got GPIO messageQueueId: %d\n", gpioMessageQueueId);
        while(1)
        {
            struct NativeGpioMsessage received;
            if (msgrcv(gpioMessageQueueId, &received, sizeof(received.pin) + sizeof(received.action), NATIVE_GPIO_MESSAGE_TYPE, 0)< 0)
            {
                PrintMessagingError(errno);
                if(errno == EINVAL || errno == EIDRM)
                {
                    printf("Re-initializing the queue\n");
                    gpioMessageQueueId = InitializeMessageQueue(NATIVE_GPIO_MESSAGE_TYPE);
                    printf("New messageQueueId: %d\n", gpioMessageQueueId);
                }
            }
            printf("Received native GPIO action %d with pin %d\n", received.action, received.pin);

            InitLed(received.pin);
            if(received.action == LED_ON)
            {
                TurnNormalLedOn(received.pin);
            }
            if(received.action == LED_OFF)
            {
                TurnNormalLedOff(received.pin);
            }
        }
        msgctl(gpioMessageQueueId, IPC_RMID, NULL);
    }
    printf("GPIO thread exits.\n");
}