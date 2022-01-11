#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "pins-mapping.h"

#define DEFAULT_SERVO_PIN 18
#define DEFAULT_LASER_PIN 4
#define LEDS_COUNT 7

static const char PINS_MAPPING_FILE_NAME[] = "pins-mapping.csv"; // File name with the mapping settings

// Read file Test
// int main()
// {
//     int servoPin = ServoPinNumber();
//     int laserPin = LaserPinNumber();
//     printf("ServoPin is %d and laserPin is %d\n", servoPin, laserPin);

//     struct PinMapping* ledPins = GetLedsMapping();
//     for(int i = 0; i < LEDS_COUNT; i++)
//     {
//         printf("Led number %d, red pin %d; green pin %d \n", ledPins[i].ledNumber, ledPins[i].redPin, ledPins[i].greenPin);
//     }

//     free(ledPins);
// }

/*
* Reads leds pin mappings from csv
*/
struct PinMapping* GetLedsMapping()
{
    struct PinMapping* pins = malloc(LEDS_COUNT * sizeof(struct PinMapping));

    FILE *filePointer = fopen(PINS_MAPPING_FILE_NAME, "r");
    if(filePointer == NULL)
    {
        return NULL;
    }

    char device[4];
    int ledNumber;
    int redPinNumber;
    int greenPinNumber;
    int rowResult;

    int i = 0;

    while ((rowResult = fscanf(filePointer, "%3s;%d;%d;%d\n", device, &ledNumber, &redPinNumber, &greenPinNumber)) != EOF)
    {
        if(rowResult >= 4 && strcmp(device, "LED") == 0)
        {
            pins[i].ledNumber = ledNumber;
            pins[i].redPin = redPinNumber;
            pins[i].greenPin = greenPinNumber;
            i++;
        }
        if(i >= LEDS_COUNT)
        {
            break;
        }
    }
    fclose(filePointer);

    return pins;
}

int FindDeviceAndReturnRedPinNumber(char* deviceToFind, int defaultPin)
{
    FILE *filePointer = fopen(PINS_MAPPING_FILE_NAME, "r");
    if(filePointer == NULL)
    {
        return defaultPin;
    }

    char device[4];
    int ledNumber;
    int redPinNumber;
    int greenPinNumber;
    int rowResult;

    int servoPin = defaultPin;

    while ((rowResult = fscanf(filePointer, "%3s;%d;%d;%d\n", device, &ledNumber, &redPinNumber, &greenPinNumber)) != EOF)
    {
        if(rowResult >= 4 && strcmp(device, deviceToFind) == 0)
        {
            servoPin = redPinNumber;
            break;
        }
    }
    fclose(filePointer);
    return servoPin;
}

/*
* Reads pin number for Servo Motor from csv file
*/
int ServoPinNumber()
{
    return FindDeviceAndReturnRedPinNumber("SER", DEFAULT_SERVO_PIN);
}

/*
* Reads pin number for Laser Motor from csv file
*/
int LaserPinNumber()
{
    return FindDeviceAndReturnRedPinNumber("LAS", DEFAULT_LASER_PIN);
}