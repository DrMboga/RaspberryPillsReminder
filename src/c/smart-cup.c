#include <errno.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <wiringPiI2C.h>

#include "smart-cup.h"
#include "messaging.h"
#include "utils.h"

#define LASER_PIN 4

int main(int argc, char *argv[])
{
    // Switch Laser off
    int gpioMessageQueueId = InitializeMessageQueue(NATIVE_GPIO_MESSAGE_TYPE);
    if(gpioMessageQueueId >= 0)
    {
        printf("Successfully got GPIO messageQueueId: %d\n", gpioMessageQueueId);
        switchLaserOff(gpioMessageQueueId);
    }

    if(argc > 1)
    {
        if(strcmp(argv[1], "start") == 0)
        {
            StartMainProcess(gpioMessageQueueId);
            return 0;
        }
        if(strcmp(argv[1], "tune") == 0)
        {
            StartTuningMode(gpioMessageQueueId);
            return 0;
        }
    }

    // Initialize Message queue
    int messageQueueId = InitializeMessageQueue(SMART_CUP_CONTROL_MESSAGE_TYPE);
    
    if(messageQueueId >= 0)
    {
        printf("Successfully got messageQueueId: %d\n", messageQueueId);
        while(1) 
        {
            printf("Waiting for incoming message...\n");
            struct SmartCupProcessStartMsessage received;
            // If no LEDs is blinking, current thread is sleeping while waiting new message from the queue
            if (msgrcv(messageQueueId, &received, sizeof(received.flag), SMART_CUP_CONTROL_MESSAGE_TYPE, 0)< 0)
            {
                PrintMessagingError(errno);
                if(errno == EINVAL || errno == EIDRM)
                {
                    printf("Re-initializing the queue\n");
                    messageQueueId = InitializeMessageQueue(SMART_CUP_CONTROL_MESSAGE_TYPE);
                    printf("New messageQueueId: %d\n", messageQueueId);
                    if(messageQueueId == -1)
                    {
                        return 1;
                    }
                }
            }
            printf("Received new message to start smart cup main process\n");

            StartMainProcess(gpioMessageQueueId);
        }
        msgctl(messageQueueId, IPC_RMID, NULL);
    }

    return 0;
}

void StartMainProcess(int gpioMessageQueueId)
{

}

void StartTuningMode(int gpioMessageQueueId)
{
    int wiringPiHandle = wiringPiI2CSetup(0x23);

    for (int i = 0; i < 10; i++)
    {
        // Turn Laser on
        switchLaserOn(gpioMessageQueueId);
        sleepMilliseconds(100);

        for (int i = 0; i < 10; i++)
        {
            // Check the sensor
            wiringPiI2CWrite(wiringPiHandle,0x10);
            sleepMilliseconds(200);
            int word=wiringPiI2CReadReg16(wiringPiHandle,0x00);
            int lux=((word & 0xff00)>>8) | ((word & 0x00ff)<<8);

            // curent time
            char currentTime[10];
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            strftime(currentTime, 10, "%H:%M.%S", timeinfo);

            printf("[%s] Current illuminance in lux:%d \n", currentTime, lux);
            sleepMilliseconds(500);
        }
        

        // Turn Laser off
        switchLaserOff(gpioMessageQueueId);
        sleepMilliseconds(1000);
    }
}

void switchLaserOn(int gpioMessageQueueId)
{
    struct NativeGpioMsessage gpioMessage;
    gpioMessage.mtype = NATIVE_GPIO_MESSAGE_TYPE;
    gpioMessage.pin = LASER_PIN;
    gpioMessage.action = LED_ON;
    if(msgsnd(gpioMessageQueueId, (void*)&gpioMessage, sizeof(gpioMessage.pin) + sizeof(gpioMessage.action), 0) < 0)
    {
        printf("msgsnd error !!\n");
    }
}
void switchLaserOff(int gpioMessageQueueId)
{
    struct NativeGpioMsessage gpioMessage;
    gpioMessage.mtype = NATIVE_GPIO_MESSAGE_TYPE;
    gpioMessage.pin = LASER_PIN;
    gpioMessage.action = LED_OFF;
    if(msgsnd(gpioMessageQueueId, (void*)&gpioMessage, sizeof(gpioMessage.pin) + sizeof(gpioMessage.action), 0) < 0)
    {
        printf("msgsnd error !!\n");
    }
}