static const char CUP_STATE_FILE_NAME[] = "cup-state.csv"; // File name with the cup state
static const char ANGLES_MAP_FILE_NAME[] = "angles.csv";   // File name with the servo angles mapping
static const char REPORT_FILE_NAME[] = "report.csv";       // File name with the report

#define PILLS_COUNT 7

#define CUP_STATE_PILL_INITIAL 0                         // No actions have been made yet
#define CUP_STATE_PILL_DROPPED 1                         // Signal to move servo has sent and signal to blink the LED has sent
#define CUP_STATE_PILL_DROPPED_AND_INSIDE 2              // After moving servo, the pill should be in the cup
#define CUP_ERROR_STATE_PILL_DROPPED_BUT_CUP_IS_EMPTY -3 // Error state - the servo has moved but the pill has stuck somewhere
#define CUP_STATE_PILL_TAKEN 4                           // Pill has taken, signal to turn green LED has sent, the row has added to the journal
#define CUP_ERROR_STATE_PILL_NOT_TAKEN -5                // Error state - at 20:00 pill is still in the cup, the signal to turn red LED has sent, the row has added to the journal

#define GATE_ILLUMINANCE 50000 // The border value for light sensor. If current illuminance is greater then this value or equal, it means that laser is directly points to the sensor

#define STOP_HOUR 20

/*
 * Csv smart cup state file data row structure
 */
struct CupStateRow
{
    int pillNumber;
    int state;
    char time[21];
};

/*
 * Csv with mapping the pill number with servo angle
 */
struct ServoAngleMap
{
    int pillNumber;
    int angle;
};

void StartMainProcess(int gpioMessageQueueId);
void StartTuningMode(int gpioMessageQueueId);

void switchLaserOn(int gpioMessageQueueId);
void switchLaserOff(int gpioMessageQueueId);

/*
 * Reads Cup state from csv file
 */
struct CupStateRow *ReadCupState();

/*
 * Writes a new Cup state into csv file
 */
int RewriteCupState(struct CupStateRow *ledsState);

/*
 * Desides which action to perform with the cup current state
 */
int MoveToTheNextState(int currentState, int pillNumber, int gpioMessageQueueId);

/*
 * Sends command to move servo.
 */
void MoveServo(int pillNumber);

/*
 * Reads angles mappng from csv file
 */
struct ServoAngleMap *ReadServoMapping();

/*
 * Sends command to turn servo motor
 */
int sendServoMessage(int angle);

/*
 * Sends command to blink LED red
 */
int sendCommandToBlinkRedLED(int ledNumber);

/*
 * Sends command to turn on LED red
 */
int sendCommandToTurnRedLED(int ledNumber);

/*
 * Sends command to turn on LED green
 */
int sendCommandToTurnGreenLED(int ledNumber);

int sendLEDMessage(int ledNumber, int action, int color);

/*
 * Checks if cup is empty
 */
int isCupEmpty(int gpioMessageQueueId);

/*
 * Adds a row to report file
 */
void saveReportRow(int pillNumber, int successfully);
