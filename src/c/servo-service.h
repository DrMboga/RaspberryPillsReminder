static const char SERVO_STATE_FILE_NAME[] = "servo-state.csv"; // File name with the Servo angle state


/*
* Reads Servo angle from csv file
*/
int ReadServoState();

/*
* Turns the servo motor
*/
void SetServoAngle(int angle);

/*
* Turns the servo motor slowly
*/
void SetServoAngleSlowly(int initialAngle, int desiredAngle);

/*
* Writes a new angle value to the file
*/
void RewriteServoState(int angle);