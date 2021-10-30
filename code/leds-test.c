// gcc messaging.c leds-test.c -o leds-test

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "messaging.h"

int main()
{
    int messageQueueId = InitializeMessageQueue();
    
    if(messageQueueId == -1)
    {
        return -1;
    }
    printf("Successfully got messageQueueId: %d\n", messageQueueId);

    int ledNumber = 14;
    int currentSatatus = LED_OFF;
    while(1) {
        char ledStatus[28];
        switch (currentSatatus)
        {
        case LED_OFF:
            strcpy(ledStatus, "off");
            break;
        case LED_BLINK:
            strcpy(ledStatus, "blinking");
            break;
        case LED_ON:
            strcpy(ledStatus, "on");
            break;
        default:
            break;
        }
        printf("Led %d is %s: \n", ledNumber, ledStatus);

        printf("Please enter new LED pin: ");
        scanf("%d/n", &ledNumber);
        printf("Please type 'b' for blink, 's' for switch off, 'o' for on, 'e' for exit ");
        char answer[2];
        scanf("%1s/n", answer);
        if(strcmp(answer, "e") == 0)
        {
            break;
        }
        if(strcmp(answer, "b")== 0)
        {
            currentSatatus = LED_BLINK;
        }
        if(strcmp(answer, "s")== 0)
        {
            currentSatatus = LED_OFF;
        }
        if(strcmp(answer, "o")== 0)
        {
            currentSatatus = LED_ON;
        }

        struct LedsMsg ledMessage;
        ledMessage.mtype = LED_CONTROL_MESSAGE_TYPE;
        ledMessage.LedAction = currentSatatus;
        ledMessage.LedPin = ledNumber;
        if(msgsnd(messageQueueId, (void*)&ledMessage, sizeof(ledMessage.LedAction) + sizeof(ledMessage.LedPin), 0) < 0){
            printf("msgsnd error !!\n");
        }
    }
    msgctl(messageQueueId, IPC_RMID, NULL);

    return 0;
}