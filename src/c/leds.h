/* Initializes GPIO*/
int InitLed(int pin);

/* Bi color LED uses common anode. So, to switch it on we used to set 0 to GPIO pin*/
void TurnBiColorLedOn(int pin);

/* Bi color LED uses common anode. So, to switch it off we used to set 1 to GPIO pin*/
void TurnBiColorLedOff(int pin);

/* Set 1 to GPIO pin*/
void TurnNormalLedOn(int pin);

/* Set 0 to GPIO pin*/
void TurnNormalLedOff(int pin);
