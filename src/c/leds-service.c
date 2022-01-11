#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "leds-service.h"
#include "file-access.h"
#include "leds.h"

#define GREEN 42
#define RED 24
#define ON 142
#define OFF 124
#define BLINK 100

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


    free(ledsState);
    free(ledPins);
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
            case LED_OFF:
                action = OFF;
                break;
            case LED_GREEN_ON:
                action = ON;
                color = GREEN;
                break;
            case LED_GREEN_BLINK:
                action = BLINK;
                color = GREEN;
                break;
            case LED_RED_ON:
                action = ON;
                color = RED;
                break;
            case LED_RED_BLINK:
                action = BLINK;
                color = RED;
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
            switch(action)
            {
                case OFF:
                    InitLed(ledPins[j].redPin);
                    TurnBiColorLedOff(ledPins[j].redPin);
                    InitLed(ledPins[j].greenPin);
                    TurnBiColorLedOff(ledPins[j].greenPin);
                    break;
                case ON:
                    switch(color)
                    {
                        case GREEN:
                            InitLed(ledPins[j].redPin);
                            TurnBiColorLedOn(ledPins[j].redPin);
                            break;
                        case RED:
                            InitLed(ledPins[j].greenPin);
                            TurnBiColorLedOn(ledPins[j].greenPin);
                            break;
                    }
                    break;
                case BLINK:
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
            leds[i].state = LED_OFF;
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
int RewriteLedsState(struct LedStateRow* ledsState)
{
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