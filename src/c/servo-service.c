#include <wiringPi.h>
#include <softPwm.h>  /* include header file for software PWM */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include "servo-service.h"
#include "utils.h"
#include "messaging.h"
#include "file-access.h"

/* GPIO 1 as per WiringPi, GPIO18 as per BCM */
#define PWM_pin 1 

#define DEFAULT_ANGLE 180

int main()
{
    // Reading servo angle from csv file
    int servoAngle = ReadServoState();

    if (wiringPiSetup () == -1)
        exit (1) ;

    pinMode (PWM_pin, PWM_OUTPUT) ; /* set PWM pin as output */

    softPwmCreate(PWM_pin,1,25);
    SetServoAngle(servoAngle);

    printf("Wiring pi has set up, servo motor has angle %d\n", servoAngle);

    // Initialize Message queue
    int messageQueueId = InitializeMessageQueue(SERVO_CONTROL_MESSAGE_TYPE);
    
    if(messageQueueId >= 0)
    {
        printf("Successfully got messageQueueId: %d\n", messageQueueId);
        while(1) 
        {
            printf("Waiting for incoming message...\n");
            struct ServoControlMsessage received;
            // If no LEDs is blinking, current thread is sleeping while waiting new message from the queue
            if (msgrcv(messageQueueId, &received, sizeof(received.angle), SERVO_CONTROL_MESSAGE_TYPE, 0)< 0)
            {
                PrintMessagingError(errno);
                if(errno == EINVAL || errno == EIDRM)
                {
                    printf("Re-initializing the queue\n");
                    messageQueueId = InitializeMessageQueue(SERVO_CONTROL_MESSAGE_TYPE);
                    printf("New messageQueueId: %d\n", messageQueueId);
                    if(messageQueueId == -1)
                    {
                        return 1;
                    }
                }
            }
            printf("Received new message to turn servo motor on angle %d\n", received.angle);

            SetServoAngle(received.angle);
            RewriteServoState(received.angle);
        }
        msgctl(messageQueueId, IPC_RMID, NULL);
    }

    return 0;
}

/*
* Reads Servo angle from csv file
*/
int ReadServoState()
{
    FILE* servoStateFilePointer = OpenFileForRead(SERVO_STATE_FILE_NAME);
    if(servoStateFilePointer == NULL)
    {
        // File doesn't exist yet
        return DEFAULT_ANGLE;
    }

    int rowResult;
    int angle;
    while ((rowResult = fscanf(servoStateFilePointer, "%d\n", &angle)) != EOF)
    {
        if(rowResult >= 1)
        {
            break;
        }
    }
    return angle;

    CloseFile(servoStateFilePointer);
}

/*
* Writes a new angle value to the file
*/
void RewriteServoState(int angle)
{
    FILE* servoStateFilePointer = OpenFileForRewrite(SERVO_STATE_FILE_NAME);

    if(fprintf(servoStateFilePointer, "%d\n", angle) < 0)
    {
        printf("Write operaion failed");
    }
    CloseFile(servoStateFilePointer);
}

/*
* Turns the servo motor
*/
void SetServoAngle(int angle)
{
    int intensity = 0;
    if( angle >= 0 && angle <= 180) 
    {
        intensity = (int) (4 + (angle * 20 / 180));
        // printf("Intensity %d: \n", intensity);
        softPwmWrite (PWM_pin, intensity);
        sleepMilliseconds(300);
        softPwmWrite (PWM_pin, 0);
    }
}