#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include <time.h>
#include "leds-service.h"
#include "file-access.h"
#include "leds.h"
#include "messaging.h"

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

    // Initialize Message qeue
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
            printf("Received message with action %d and led number %d:\n", received.LedAction, received.LedNumber);

            if(received.LedAction == LED_ON || received.LedAction == LED_OFF)
            {
                applyLedAction(received.LedNumber, received.LedColor, received.LedAction, ledPins);
                RewriteLedsState(received.LedNumber, received.LedColor, received.LedAction, ledsState);
            }

            // if(received.LedAction == LED_BLINK)
            // {
            //     int blinkingLedPin = received.LedPin;
            //     if(InitLed(blinkingLedPin) < 0)
            //     {
            //         printf("InitLed failed.\n");
            //         return 1;
            //     }
            //     printf("Start blinking...\n");
            //     int continueBlinking = 1;
            //     while (continueBlinking)
            //     {
            //         printf("--Led On!--\n");
            //         TurnBiColorLedOn(blinkingLedPin);
            //         sleepMilliseconds(500);
            //         printf("--Led Off!--\n");
            //         TurnBiColorLedOff(blinkingLedPin);

            //         // While blinking, there is no need to freeze thread while checking the message in the queue
            //         if (msgrcv(messageQueueId, &received, sizeof(received.LedAction) + sizeof(received.LedPin), LED_CONTROL_MESSAGE_TYPE, IPC_NOWAIT)< 0){
            //             if(errno == ENOMSG)
            //             {
            //                 printf("No message in the queue, sleeping\n");
            //                 sleepMilliseconds(500);
            //             } else 
            //             {
            //                 PrintMessagingError(errno);
            //                 if(InitLeds() < 0)
            //                 {
            //                     return 1;
            //                 }
            //                 if(errno == EINVAL || errno == EIDRM)
            //                 {
            //                     printf("Re-initializing the queue\n");
            //                     messageQueueId = InitializeMessageQueue();
            //                     printf("New messageQueueId: %d\n", messageQueueId);
            //                     if(messageQueueId == -1)
            //                     {
            //                         return 1;
            //                     }
            //                 }
            //                 else
            //                 {
            //                     return 1;
            //                 }
            //                 continueBlinking = 0;
            //             }
            //         } else {
            //             printf("Received message in the second cycle with action %d and led PIN number %d:\n", received.LedAction, received.LedPin);
            //             if(received.LedPin == blinkingLedPin && received.LedAction != LED_BLINK)
            //             {
            //                 continueBlinking = 0;
            //             }
            //             if(received.LedPin != blinkingLedPin && received.LedAction == LED_BLINK)
            //             {
            //                 // Change blinking LED
            //                 if(SwitchLed(blinkingLedPin, LED_OFF) < 0)
            //                 {
            //                     return 1;
            //                 }
            //                 blinkingLedPin = received.LedPin;
            //             }
            //             if(received.LedAction == LED_ON || received.LedAction == LED_OFF)
            //             {
            //                 if(SwitchLed(received.LedPin, received.LedAction) < 0)
            //                 {
            //                     return 1;
            //                 }
            //             }
            //         }
            //     }
            // }
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
                    // TODO: setup blink in the separate thread
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