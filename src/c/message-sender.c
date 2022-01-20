#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "messaging.h"

int sendLedMessage(int ledNumber, char* color[], char* action[]);
int sendServoMessage(int angle);

int main(int argc, char *argv[])
{
    // LED 0 GREEN ON
    if(argc < 2)
    {
        printf("Expected arguments like 'led 0 green on'\n");
        return 0;
    }

    if(strcmp(argv[1], "led") == 0)
    {
        if(argc != 5)
        {
            printf("Insufficient number of arguments\n");
            return 0;
        }
        int ledNumber = (int) (argv[2][0] - '0');
        if (ledNumber < 0 || ledNumber > 6)
        {
            printf("Wrong LED number %d. It should be between 0 and 6\n", ledNumber);
            return 0;
        }
        if (strcmp(argv[3], "green") != 0 && strcmp(argv[3], "red"))
        {
            printf("Wrong LED color '%s'. It should be only 'green' or 'red'\n", argv[3]);
            return 0;
        }
        if (strcmp(argv[4], "on") != 0 && strcmp(argv[4], "off") && strcmp(argv[4], "blink"))
        {
            printf("Wrong LED action '%s'. It should be only 'on' or 'off' or 'blink'\n", argv[4]);
            return 0;
        }
        if(sendLedMessage(ledNumber, &argv[3], &argv[4]) < 0)
        {
            perror("Could not send message to the queue!");
        }
    }

    if(strcmp(argv[1], "servo") == 0)
    {
        if(argc != 3)
        {
            printf("Insufficient number of arguments\n");
            return 0;
        }
        int angle = atoi(argv[2]);
        if(angle < 0 || angle > 180)
        {
            printf("Wrong angle %d. It should be between 0 and 180\n", angle);
            return 0;
        }
        if(sendServoMessage(angle) < 0)
        {
            perror("Could not send message to the queue!");
        }
    }

    return 0;
}

int sendLedMessage(int ledNumber, char* color[], char* action[])
{
    int ledAction, ledColor;
    if(strcmp(*color, "green") == 0)
    {
        ledColor = GREEN_LED;
    }
    if(strcmp(*color, "red") == 0)
    {
        ledColor = RED_LED;
    }
    if(strcmp(*action, "on") == 0)
    {
        ledAction = LED_ON;
    }
    if(strcmp(*action, "off") == 0)
    {
        ledAction = LED_OFF;
    }
    if(strcmp(*action, "blink") == 0)
    {
        ledAction = LED_BLINK;
    }

    printf("ledColor: %d; ledAction: %d\n", ledColor, ledAction);

    int messageQueueId = InitializeMessageQueue(LED_CONTROL_MESSAGE_TYPE);
    if(messageQueueId == -1)
    {
        return -1;
    }
    printf("Successfully got messageQueueId: %d\n", messageQueueId);
    struct LedsControlMsessage ledMessage;
    ledMessage.mtype = LED_CONTROL_MESSAGE_TYPE;
    ledMessage.LedNumber = ledNumber;
    ledMessage.LedAction = ledAction;
    ledMessage.LedColor = ledColor;

    if(msgsnd(messageQueueId, (void*)&ledMessage, sizeof(ledMessage.LedAction) + sizeof(ledMessage.LedNumber) + sizeof(ledMessage.LedColor), 0) < 0){
        printf("msgsnd error !!\n");
    }

    // msgctl(messageQueueId, IPC_RMID, NULL);
    return 0;
}

int sendServoMessage(int angle)
{
    int messageQueueId = InitializeMessageQueue(SERVO_CONTROL_MESSAGE_TYPE);
    if(messageQueueId == -1)
    {
        return -1;
    }
    printf("Successfully got messageQueueId: %d\n", messageQueueId);
    struct ServoControlMsessage servoMessage;
    servoMessage.mtype = SERVO_CONTROL_MESSAGE_TYPE;
    servoMessage.angle = angle;

    if(msgsnd(messageQueueId, (void*)&servoMessage, sizeof(servoMessage.angle), 0) < 0)
    {
        printf("msgsnd error !!\n");
    }

    return 0;    
}