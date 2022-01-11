
struct PinMapping
{
    int ledNumber;
    int redPin;
    int greenPin;
};

/*
* Reads leds pin mappings from csv
*/
struct PinMapping* GetLedsMapping();

/*
* Reads pin number for Servo Motor from csv file
*/
int ServoPinNumber();

/*
* Reads pin number for Laser Motor from csv file
*/
int LaserPinNumber();
