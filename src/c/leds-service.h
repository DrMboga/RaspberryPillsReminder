#include "pins-mapping.h"

#define LEDS_COUNT 7 // Total LEDs count in a pills reminder board
static const char LEDS_STATE_FILE_NAME[] = "leds-state.csv"; // File name with the LEDs state
static const char EMPTY_TIME[] = "--:--"; // Stub for empty time in a csv report

#define LEDS_OFF 0 // LED off state
#define LED_GREEN_ON 1 // Green LED ON state
#define LED_GREEN_BLINK 2 // Green LED BLINK state
#define LED_RED_ON 4 // Red LED ON state
#define LED_RED_BLINK 8 // Red LED BLINK state

/*
* Csv LEDSs state file data row structure
*/
struct LedStateRow
{
    int ledNumber;
    int state;
    char time[6];
};

/*
* Reads LEDs state from csv file
*/
struct LedStateRow* ReadLedsState();

/*
* Writes a new LEDs state into csv file
*/
int RewriteLedsState(int ledNumber, int color, int action, struct LedStateRow* ledsState);

/*
* Sets up LEDs state in the board
*/
int SetUpLedsState(struct LedStateRow* ledsState, struct PinMapping* ledPins);

/*
* Find LED pin and applies an action to it
*/
void applyLedAction(int ledNumber, int color, int action, struct PinMapping* ledPins);
