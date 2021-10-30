#define LED_CONTROL_MESSAGE_TYPE 1 // Message type for controlling leds
#define LED_BLINK 1 // Action for blinking the LED
#define LED_OFF 2 // Action for switch LED off
#define LED_ON 3 // Action for switch LED on

struct LedsMsg
{
   long mtype;
   int LedAction;
   int LedPin;
};

/*
    Initializes the message queue and returns the message queue identifier
*/
int InitializeMessageQueue();