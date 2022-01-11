#include <pigpio.h>
// #include <wiringPi.h>

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

int InitLed(int pin)
{
    if (gpioInitialise() < 0)
    {
        printf("pigpio initialisation failed\n");
        return 1;
    }
    /* Set GPIO modes */
    gpioSetMode(pin, PI_OUTPUT);
    return 0;
}

void TurnBiColorLedOn(int pin)
{
    gpioWrite(pin, 0); // For Bi Color LED we used to set 0 to cathode pin to switch it on
}

void TurnBiColorLedOff(int pin)
{
    gpioWrite(pin, 1); // For Bi Color LED we used to set 1 to cathode pin to switch it off
}
