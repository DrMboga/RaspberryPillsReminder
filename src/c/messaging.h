#define LED_CONTROL_MESSAGE_TYPE 1 // Message type for controlling leds
#define SERVO_CONTROL_MESSAGE_TYPE 2 // Message type for controlling servo motor
#define LED_BLINK 1 // Action for blinking the LED
#define LED_OFF 2 // Action for switch LED off
#define LED_ON 3 // Action for switch LED on

#define GREEN_LED 11
#define RED_LED 12

struct LedsControlMsessage
{
   long mtype;
   int LedAction;
   int LedNumber;
   int LedColor;
};

struct ServoControlMsessage
{
   long mtype;
   int angle;
};

/*
*    Initializes the message queue and returns the message queue identifier
*/
int InitializeMessageQueue(int queueId);

/*
*    Prints an error in the message quque system
*/
void PrintMessagingError(int errorNumber);