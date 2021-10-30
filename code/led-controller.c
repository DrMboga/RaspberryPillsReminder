// scp -r ./code pi@192.168.0.67:/home/pi/projects

// gcc messaging.c leds.c utils.c led-controller.c -o led-controller -l wiringPi -lpigpio


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

#include "messaging.h"
#include "leds.h"
#include "utils.h"

/*
* Process waithing messages from Linux System V message queue.
* When message arrives, the process make necessary action and waits another action
* Restriction - Only one LED can blink at the same time.
*/

int InitLeds();

int SwitchLed(int ledNumber, int action);

void PrintError(int errorNumber);

int main()
{
    if(InitLeds() < 0)
    {
        return 1;
    }
    int messageQueueId = InitializeMessageQueue();
    
    if(messageQueueId == -1)
    {
        return 1;
    }
    printf("Successfully got messageQueueId: %d\n", messageQueueId);
    while(1) {
        printf("Waiting for incoming message...\n");
        struct LedsMsg received;
        // If no LEDs is blinking, current thread is sleeping while waiting new message from the queue
        if (msgrcv(messageQueueId, &received, sizeof(received.LedAction) + sizeof(received.LedPin), LED_CONTROL_MESSAGE_TYPE, 0)< 0){
            PrintError(errno);
            if(InitLeds() < 0)
            {
                return 1;
            }
            if(errno == EINVAL || errno == EIDRM)
            {
                printf("Re-initializing the queue\n");
                messageQueueId = InitializeMessageQueue();
                printf("New messageQueueId: %d\n", messageQueueId);
                if(messageQueueId == -1)
                {
                    return 1;
                }
            }
            else
            {
                return 1;
            }
        }
        printf("Received message with action %d and led PIN number %d:\n", received.LedAction, received.LedPin);

        if(received.LedAction == LED_ON || received.LedAction == LED_OFF)
        {
            if(SwitchLed(received.LedPin, received.LedAction) < 0)
            {
                return 1;
            }
        }

        if(received.LedAction == LED_BLINK)
        {
            int blinkingLedPin = received.LedPin;
            if(InitLed(blinkingLedPin) < 0)
            {
                printf("InitLed failed.\n");
                return 1;
            }
            printf("Start blinking...\n");
            int continueBlinking = 1;
            while (continueBlinking)
            {
                printf("--Led On!--\n");
                TurnBiColorLedOn(blinkingLedPin);
                sleepMilliseconds(500);
                printf("--Led Off!--\n");
                TurnBiColorLedOff(blinkingLedPin);

                // While blinking, there is no need to freeze thread while checking the message in the queue
                if (msgrcv(messageQueueId, &received, sizeof(received.LedAction) + sizeof(received.LedPin), LED_CONTROL_MESSAGE_TYPE, IPC_NOWAIT)< 0){
                    if(errno == ENOMSG)
                    {
                        printf("No message in the queue, sleeping\n");
                        sleepMilliseconds(500);
                    } else 
                    {
                        PrintError(errno);
                        if(InitLeds() < 0)
                        {
                            return 1;
                        }
                        if(errno == EINVAL || errno == EIDRM)
                        {
                            printf("Re-initializing the queue\n");
                            messageQueueId = InitializeMessageQueue();
                            printf("New messageQueueId: %d\n", messageQueueId);
                            if(messageQueueId == -1)
                            {
                                return 1;
                            }
                        }
                        else
                        {
                            return 1;
                        }
                        continueBlinking = 0;
                    }
                } else {
                    printf("Received message in the second cycle with action %d and led PIN number %d:\n", received.LedAction, received.LedPin);
                    if(received.LedPin == blinkingLedPin && received.LedAction != LED_BLINK)
                    {
                        continueBlinking = 0;
                    }
                    if(received.LedPin != blinkingLedPin && received.LedAction == LED_BLINK)
                    {
                        // Change blinking LED
                        if(SwitchLed(blinkingLedPin, LED_OFF) < 0)
                        {
                            return 1;
                        }
                        blinkingLedPin = received.LedPin;
                    }
                    if(received.LedAction == LED_ON || received.LedAction == LED_OFF)
                    {
                        if(SwitchLed(received.LedPin, received.LedAction) < 0)
                        {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}

int SwitchLed(int ledNumber, int action)
{
    switch (action)
    {
        case LED_ON:
            if(InitLed(ledNumber) < 0)
            {
                printf("InitLed failed.\n");
                return -1;
            }
            TurnBiColorLedOn(ledNumber);
            break;
        case LED_OFF:
            if(InitLed(ledNumber) < 0)
            {
                printf("InitLed failed.\n");
                return -1;
            }            
            TurnBiColorLedOff(ledNumber);
            break;
    }
    return 0;
}

int InitLeds()
{
    // As soon as we are using BI Leds, we should set up 1 for each GPIO pin to switch led off
    int LedPins[14] = { 
        17, 27, 
        22, 10, 
        9, 11,
        5, 6,
        13, 19,
        14, 15,
        23, 24
        };
    for (int i = 0; i < 14; i++)
    {
        if(SwitchLed(LedPins[i], LED_OFF) < 0)
        {
            return 1;
        }
    }
}

void PrintError(int errorNumber) {
    switch (errorNumber)
    {
    case E2BIG:
        printf("msgrcv error - E2BIG!!\n");
        break;
    case EACCES:
        printf("msgrcv error - EACCES!!\n");
        break;
    case EIDRM:
        printf("msgrcv error - EIDRM!!\n");
        break;
    case EINTR:
        printf("msgrcv error - EINTR!!\n");
        break;
    case EINVAL:
        printf("msgrcv error - EINVAL!!\n");
        break;
    case ENOMSG:
        printf("msgrcv error - ENOMSG!!\n");
        break;
    default:
        break;
    }
}
