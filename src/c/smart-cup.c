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
#include "file-access.h"

#define LASER_PIN 4

// If no parapeters - process waits a message for start main process, when main process done, waits another message infintely
// If parameter "start" - starts main process immediately, finishes the process after maine process done
// If parameter "tune" - starts tuning program - switch laser and check the sensor
int main(int argc, char *argv[])
{
    // Switch Laser off
    int gpioMessageQueueId = InitializeMessageQueue(NATIVE_GPIO_MESSAGE_TYPE);
    if (gpioMessageQueueId >= 0)
    {
        printf("Successfully got GPIO messageQueueId: %d\n", gpioMessageQueueId);
        switchLaserOff(gpioMessageQueueId);
    }

    if (argc > 1)
    {
        if (strcmp(argv[1], "start") == 0)
        {
            StartMainProcess(gpioMessageQueueId);
            return 0;
        }
        if (strcmp(argv[1], "tune") == 0)
        {
            StartTuningMode(gpioMessageQueueId);
            return 0;
        }
    }

    // Initialize Message queue
    int messageQueueId = InitializeMessageQueue(SMART_CUP_CONTROL_MESSAGE_TYPE);

    if (messageQueueId >= 0)
    {
        printf("Successfully got messageQueueId: %d\n", messageQueueId);
        while (1)
        {
            printf("Waiting for incoming message...\n");
            struct SmartCupProcessStartMsessage received;
            // If no LEDs is blinking, current thread is sleeping while waiting new message from the queue
            if (msgrcv(messageQueueId, &received, sizeof(received.flag), SMART_CUP_CONTROL_MESSAGE_TYPE, 0) < 0)
            {
                PrintMessagingError(errno);
                if (errno == EINVAL || errno == EIDRM)
                {
                    printf("Re-initializing the queue\n");
                    messageQueueId = InitializeMessageQueue(SMART_CUP_CONTROL_MESSAGE_TYPE);
                    printf("New messageQueueId: %d\n", messageQueueId);
                    if (messageQueueId == -1)
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

#pragma region Main process

void StartMainProcess(int gpioMessageQueueId)
{
    // Reading cup state from csv file
    struct CupStateRow *cupState = ReadCupState();
    printf("Current pill is %d, state %d, was set up %s\n", cupState->pillNumber, cupState->state, cupState->time);

    while (1)
    {
        int nextState = MoveToTheNextState(cupState->state, cupState->pillNumber, gpioMessageQueueId);
        printf("Action to perform %d \n", nextState);
        char *todayString = today();
        strcpy(cupState->time, todayString);
        cupState->state = nextState;
        if (nextState == CUP_STATE_PILL_TAKEN)
        {
            printf("Pill is taken \n");
            sendCommandToTurnGreenLED(cupState->pillNumber);
            saveReportRow(cupState->pillNumber, 1);
            cupState->state = CUP_STATE_PILL_INITIAL;
            if (cupState->pillNumber == PILLS_COUNT - 1)
            {
                cupState->pillNumber = 0;
            }
            else
            {
                cupState->pillNumber = cupState->pillNumber + 1;
            }
            RewriteCupState(cupState);

            break;
        }
        else
        {
            RewriteCupState(cupState);
            // TODO: check current time. If it is 20:00, then write error to journal, send red light to LED
        }
        sleepMilliseconds(1000);
    }

    free(cupState);
}

/*
 * Desides which action to perform with the cup current state
 */
int MoveToTheNextState(int currentState, int pillNumber, int gpioMessageQueueId)
{
    int isEmpty;
    switch (currentState)
    {
    case CUP_STATE_PILL_INITIAL:
        printf("Current state is 'initial'. Pill number %d\n", pillNumber);
        MoveServo(pillNumber);
        sendCommandToBlinkRedLED(pillNumber);
        isEmpty = isCupEmpty(gpioMessageQueueId);
        if (isEmpty)
        {
            return CUP_ERROR_STATE_PILL_DROPPED_BUT_CUP_IS_EMPTY;
        }
        return CUP_STATE_PILL_DROPPED_AND_INSIDE;
    case CUP_STATE_PILL_DROPPED_AND_INSIDE:
        printf("Current state is 'pill is inside'. Pill number %d\n", pillNumber);
        isEmpty = isCupEmpty(gpioMessageQueueId);
        if (isEmpty)
        {
            return CUP_STATE_PILL_TAKEN;
        }
        return CUP_STATE_PILL_DROPPED_AND_INSIDE;
        break;
    case CUP_ERROR_STATE_PILL_DROPPED_BUT_CUP_IS_EMPTY:
        printf("Current state is 'error - pill should be inside'. Pill number %d\n", pillNumber);
        isEmpty = isCupEmpty(gpioMessageQueueId);
        if (isEmpty)
        {
            return CUP_ERROR_STATE_PILL_DROPPED_BUT_CUP_IS_EMPTY;
        }
        return CUP_STATE_PILL_DROPPED_AND_INSIDE;
    default: // TODO: add state if previous day pill is not taken
        break;
    }
}

/*
 * Sends command to move servo.
 */
void MoveServo(int pillNumber)
{
    struct ServoAngleMap *servoMapping = ReadServoMapping();
    for (int i = 0; i < PILLS_COUNT; i++)
    {
        if (servoMapping[i].pillNumber == pillNumber)
        {
            sendServoMessage(servoMapping[i].angle);
            printf("Message to move servo on %d degrees has sent\n", servoMapping[i].angle);
            sleepMilliseconds(500);
            break;
        }
    }

    free(servoMapping);
}
#pragma endregion

#pragma region Tuning mode
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
            wiringPiI2CWrite(wiringPiHandle, 0x10);
            sleepMilliseconds(200);
            int word = wiringPiI2CReadReg16(wiringPiHandle, 0x00);
            int lux = ((word & 0xff00) >> 8) | ((word & 0x00ff) << 8);

            char *currentTime = nowTimeWithSeconds();

            printf("[%s] Current illuminance in lux:%d \n", currentTime, lux);
            free(currentTime);
            sleepMilliseconds(500);
        }

        // Turn Laser off
        switchLaserOff(gpioMessageQueueId);
        sleepMilliseconds(1000);
    }
}
#pragma endregion

#pragma region Laser control
void switchLaserOn(int gpioMessageQueueId)
{
    struct NativeGpioMsessage gpioMessage;
    gpioMessage.mtype = NATIVE_GPIO_MESSAGE_TYPE;
    gpioMessage.pin = LASER_PIN;
    gpioMessage.action = LED_ON;
    if (msgsnd(gpioMessageQueueId, (void *)&gpioMessage, sizeof(gpioMessage.pin) + sizeof(gpioMessage.action), 0) < 0)
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
    if (msgsnd(gpioMessageQueueId, (void *)&gpioMessage, sizeof(gpioMessage.pin) + sizeof(gpioMessage.action), 0) < 0)
    {
        printf("msgsnd error !!\n");
    }
}
#pragma endregion

#pragma region File IO stuff
/*
 * Reads Cup state from csv file
 */
struct CupStateRow *ReadCupState()
{
    int pillNumber;
    int state;
    char time[21];
    int rowResult;

    struct CupStateRow *cupState = malloc(sizeof(struct CupStateRow));

    FILE *cupStateFilePointer = OpenFileForRead(CUP_STATE_FILE_NAME);

    char *todayString = today();

    if (cupStateFilePointer == NULL)
    {
        // File doesn't exist yet
        cupState->pillNumber = 0;
        cupState->state = CUP_STATE_PILL_INITIAL;
        strcpy(cupState->time, todayString);
        return cupState;
    }

    if ((rowResult = fscanf(cupStateFilePointer, "%d\t%d\t%[^\n]\n", &pillNumber, &state, time)) != EOF)
    {
        printf("Time read: %s\n", time);
        if (rowResult >= 3)
        {
            cupState->pillNumber = pillNumber;
            cupState->state = state;
            strcpy(cupState->time, time);
        }
    }

    free(todayString);
    CloseFile(cupStateFilePointer);

    return cupState;
}

/*
 * Writes a new Cup state into csv file
 */
int RewriteCupState(struct CupStateRow *cupState)
{
    FILE *cupStateFilePointer = OpenFileForRewrite(CUP_STATE_FILE_NAME);

    if (fprintf(cupStateFilePointer, "%d\t%d\t%21s\n", cupState->pillNumber, cupState->state, cupState->time) < 0)
    {
        printf("Write operaion failed");
        return -1;
    }
    return CloseFile(cupStateFilePointer);
}

/*
 * Reads angles mappng from csv file
 */
struct ServoAngleMap *ReadServoMapping()
{
    int pillNumber;
    int angle;
    int rowResult;
    int pillIndex = 0;
    struct ServoAngleMap *angles = malloc(PILLS_COUNT * sizeof(struct ServoAngleMap));

    FILE *anglesFilePointer = OpenFileForRead(ANGLES_MAP_FILE_NAME);
    if (anglesFilePointer == NULL)
    {
        // File doesn't exist
        for (int i = 0; i < PILLS_COUNT; i++)
        {
            angles[i].pillNumber = PILLS_COUNT - 1 - i;
            angles[i].angle = 180;
        }

        return angles;
    }

    while ((rowResult = fscanf(anglesFilePointer, "%d\t%d\n", &pillNumber, &angle)) != EOF)
    {
        if (rowResult >= 2)
        {
            angles[pillIndex].pillNumber = pillNumber;
            angles[pillIndex].angle = angle;
            pillIndex++;
        }
        if (pillIndex >= PILLS_COUNT)
        {
            break;
        }
    }

    CloseFile(anglesFilePointer);

    return angles;
}

/*
 * Adds a row to report file
 */
void saveReportRow(int pillNumber, int successfully)
{
    FILE *reportFilePointer = OpenFileForAppend(REPORT_FILE_NAME);
    char *todayString = today();

    if (fprintf(reportFilePointer, "%d\t%d\t%21s\n", pillNumber, successfully, todayString) < 0)
    {
        printf("Write operaion failed");
    }
    CloseFile(reportFilePointer);
}
#pragma endregion

#pragma region Servo control
/*
 * Sends command to turn servo motor
 */
int sendServoMessage(int angle)
{
    int messageQueueId = InitializeMessageQueue(SERVO_CONTROL_MESSAGE_TYPE);
    if (messageQueueId == -1)
    {
        return -1;
    }
    printf("Successfully got messageQueueId: %d\n", messageQueueId);
    struct ServoControlMsessage servoMessage;
    servoMessage.mtype = SERVO_CONTROL_MESSAGE_TYPE;
    servoMessage.angle = angle;

    if (msgsnd(messageQueueId, (void *)&servoMessage, sizeof(servoMessage.angle), 0) < 0)
    {
        printf("msgsnd error !!\n");
        return -1;
    }

    return 0;
}
#pragma endregion

#pragma region LED control

/*
 * Sends command to blink LED red
 */
int sendCommandToBlinkRedLED(int ledNumber)
{
    return sendLEDMessage(ledNumber, LED_BLINK, RED_LED);
}

/*
 * Sends command to turn on LED green
 */
int sendCommandToTurnGreenLED(int ledNumber)
{
    return sendLEDMessage(ledNumber, LED_ON, GREEN_LED);
}

int sendLEDMessage(int ledNumber, int action, int color)
{
    int messageQueueId = InitializeMessageQueue(LED_CONTROL_MESSAGE_TYPE);
    if (messageQueueId == -1)
    {
        return -1;
    }
    printf("Successfully got messageQueueId: %d\n", messageQueueId);
    struct LedsControlMsessage ledMessage;
    ledMessage.mtype = LED_CONTROL_MESSAGE_TYPE;
    ledMessage.LedNumber = ledNumber;
    ledMessage.LedAction = action;
    ledMessage.LedColor = color;

    if (msgsnd(messageQueueId, (void *)&ledMessage, sizeof(ledMessage.LedAction) + sizeof(ledMessage.LedNumber) + sizeof(ledMessage.LedColor), 0) < 0)
    {
        printf("msgsnd error !!\n");
        return -1;
    }

    // msgctl(messageQueueId, IPC_RMID, NULL);
    return 0;
}

#pragma endregion

#pragma region Cup logic
/*
 * Checks if cup is empty
 */
int isCupEmpty(int gpioMessageQueueId)
{
    int laserTurnsOnCount = 5;
    int sensorChecksCount = 5;
    int luxGateOverheadedTimes = 0;
    int wiringPiHandle = wiringPiI2CSetup(0x23);

    for (int i = 0; i < laserTurnsOnCount; i++)
    {
        // Turn Laser on
        switchLaserOn(gpioMessageQueueId);
        sleepMilliseconds(100);

        for (int i = 0; i < sensorChecksCount; i++)
        {
            // Check the sensor
            wiringPiI2CWrite(wiringPiHandle, 0x10);
            sleepMilliseconds(200);
            int word = wiringPiI2CReadReg16(wiringPiHandle, 0x00);
            int lux = ((word & 0xff00) >> 8) | ((word & 0x00ff) << 8);

            if (lux >= GATE_ILLUMINANCE)
            {
                luxGateOverheadedTimes++;
            }
            printf("Current illuminance in lux:%d \n", lux);
        }

        // Turn Laser off
        switchLaserOff(gpioMessageQueueId);
        sleepMilliseconds(1000);
    }

    // If sensor fixed illuminance greater then gate more than 70% times, then the cup is empty.
    double seventyPercent = ((double)laserTurnsOnCount) * ((double)sensorChecksCount) * 0.7;
    if (((double)luxGateOverheadedTimes) >= seventyPercent)
    {
        printf("Cup is empty \n");
        return 1; // cup is empty
    }
    printf("Cup is not empty \n");
    return 0; // cup is not empty
}
#pragma endregion
