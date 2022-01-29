static const char CUP_STATE_FILE_NAME[] = "cup-state.csv"; // File name with the LEDs state

#define CUP_STATE_PILL_INITIAL 0 // Signal to move servo has sent and signal to blink the LED has sent
#define CUP_STATE_PILL_DROPPED 1 // Signal to move servo has sent and signal to blink the LED has sent
#define CUP_STATE_PILL_DROPPED_AND_INSIDE 2 // After moving servo, the pill should be in the cup
#define CUP_ERROR_STATE_PILL_DROPPED_BUT_CUP_IS_EMPTY -3 // Error state - the servo has moved but the pill has stuck somewhere
#define CUP_STATE_PILL_TAKEN 4 // Pill has taken, signal to turn green LED has sent, the row has added to the journal
#define CUP_ERROR_STATE_PILL_NOT_TAKEN -5 // Error state - at 20:00 pill is still in the cup, the signal to turn red LED has sent, the row has added to the journal 

/*
* Csv smart cup state file data row structure
*/
struct CupStateRow
{
    int pillNumber;
    int state;
    char time[21];
};

void StartMainProcess(int gpioMessageQueueId);
void StartTuningMode(int gpioMessageQueueId);

void switchLaserOn(int gpioMessageQueueId);
void switchLaserOff(int gpioMessageQueueId);

/*
* Reads Cup state from csv file
*/
struct CupStateRow* ReadCupState();

/*
* Writes a new Cup state into csv file
*/
int RewriteCupState(struct CupStateRow* ledsState);