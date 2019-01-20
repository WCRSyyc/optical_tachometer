/* Optical Tachometer

  Measure the time needed for some number of  sensor transitions, and calculate
  RPM that implies, assuming a single transition (in one direction) per revolution.

  Reference: https://www.instructables.com/id/Measure-RPM-DIY-Portable-Digital-Tachometer/
*/

#include<LiquidCrystal595.h>

const int RPM_POWER_PIN = 3;   // VCC for sensor
const int RPM_GROUND_PIN = 4;  // Ground for sensor
const int RPM_SENSOR_PIN = 2;  // Interrupt sense
const int STATUS_LED_PIN = 12;
// 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200
const long SERIAL_BAUD = 9600;
const unsigned long SPLASH_LENGTH = 2000;  // 2 Seconds to show startup splash screen
const unsigned long PAGE_LENGTH = 2000;  // 2 Seconds before advance to next page
const unsigned long UPDATE_THROTTLE = 500;  // 0.5 Second between RPM display changes
const unsigned long IDLE_TIMEOUT = 5000;  // milliseconds to wait for new measurement
const unsigned int UPDATE_COUNT = 5;  // Minimum revolutions to use for calculations
const unsigned int TICKS_PER_REV = 1;  // Number of transitions per revolution

LiquidCrystal595 lcd(7 ,8 , 9);  // Define pins used for LCD
// volatile byte revolutionCount;
volatile unsigned int revolutionCount;    // volatile (updated by interrupt handler)
unsigned long int maxRPM;  // Maximum RPM measurement
unsigned long measureStartTime;  // time latest measurement was started
int measureStatus = LOW;       // Status LED state (value); toggled to show working
int RPMprevLen = 0;     // Previous RPM value displayed length
bool clearLCD = true;   // Flag showing when LCD screen needs to be cleared
unsigned long idleStart = 0;  // previous time a measurement was recorded

void setup()
{
  // Serial.begin(SERIAL_BAUD);
  lcd.begin(16, 2);     // Initialize LCD screen
  lcd.setLED2Pin(HIGH); // Turn on Backlight

  // Call Interrupt handler function on each LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(RPM_SENSOR_PIN), revolutionCounter, RISING);

  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(RPM_POWER_PIN, OUTPUT);  // Setup power for the optical sensor
  pinMode(RPM_GROUND_PIN, OUTPUT);
  digitalWrite(RPM_POWER_PIN, HIGH);
  digitalWrite(RPM_GROUND_PIN, LOW);

  lcd.print(F("TACHOMETER"));   // Sketch startup title text
  lcd.setCursor(0, 1);
  lcd.print(F("- WCRS"));
  delay(SPLASH_LENGTH);  // Leave the splash screen up for awhile
  lcd.clear();

  revolutionCount = 0;
  measureStartTime = 0;
} // ./setup()

void loop()
{
  unsigned long currtime = millis(); // The current time
  unsigned long idletime = currtime - idleStart;  // Time since last measurement

  if(revolutionCount >= UPDATE_COUNT ) {  // It´s time to report a new (raw) measurement
    reportLatestRPM();
    idleStart = currtime;  // Just had a new reading; reset start idle interval to now
  }

  if(idletime > IDLE_TIMEOUT) {  // There has been no new reading for awhile
    showMaxRPM();  // Show the maximum RPM
    idleStart = currtime;
  }
} // ./loop()


/**
 * revolutionCounter Interrupt handler
 *
 * Every time the sensor goes from LOW to HIGH, this function will be called
 */
void revolutionCounter()
{
  revolutionCount++;  // Increment the number of revolutions

  // IDEA: move the toggle to the main loop; toggle if count changed; get rid of delay() calls
  if (measureStatus == LOW) {
    measureStatus = HIGH;
  } else {
    measureStatus = LOW;
  }
  digitalWrite(STATUS_LED_PIN, measureStatus);  // Toggle the status LED
} // ./ revolutionCounter()


/**
 * report latest measured RPM
 */
void reportLatestRPM()
{
  /* RPM: revolutions per minute == revolutions / minutes == rev / 60 sec == rev / 60000 milliseconds
   * ms = (millis() - measureStartTime)
   * rev = revolutions / TICKS_PER_REV
   *
   * rpm = (60000 * rev) / ms ¦ =
   *
   * rpm = (revolutions / TICKS_PER_REV) * 60000 ) / (millis() - measureStartTime)
   *
   * Something is wrong in the calculation here
  */
  // Calculate the RPM using the time needed for the number of revolutions seen
  unsigned long int measuredRPM = 30 * 1000 / (millis() - measureStartTime) * revolutionCount;
  measureStartTime = millis();  // Start a new measurement
  revolutionCount = 0;

  if(clearLCD) {  // Clear the LCD to avoid any garbage text
    lcd.clear();
    lcd.print(F("SENSOR MEASURING"));  // Title for raw measurement
    clearLCD = false;  // Prevent (additional) clearing of the screen
  }

  if(measuredRPM > maxRPM) {
    maxRPM = measuredRPM;   //  Track the maximum RPM reading seen
  }

  showRawRPM(measuredRPM);
} // ./ reportLatestRPM


/**
 * Determine the number of decimal digits needed to display an int value
 *
 * TODO test this for a range of values, including zero and negative
 *
 * @param value - integer value to count the decimal digits in
 */
unsigned int decimalDigits(int value)
{
  unsigned int digitCount = 0;
  int x = value;
  while(x != 0) {
    x /= 10;
    digitCount++;
  }
  return digitCount;
}


/** showRawRPM
 *
 * Display the latest RPM measurement
 *
 * @param rpmValue - revolutions per minute value
 */
void showRawRPM(int rpmValue)
{
  int RPMcurLen = decimalDigits(rpmValue);  // Current RPM value displayed length
  if(RPMcurLen < RPMprevLen) {  // Fewer digits in the RPM display value than before
    lcd.clear();
    RPMprevLen = RPMcurLen;
    clearLCD = false;
    lcd.print(F("SENSOR MEASURING"));
  }

  lcd.setCursor(0, 1);
  lcd.print(rpmValue, DEC);  // Show calculated RPM as a decimal value

  lcd.setCursor(6,1);
  lcd.print(F("RPM"));
  delay(UPDATE_THROTTLE);  // Make sure the display stays still long enough to read
} // ./showRawRPM()


/** showMaxRPM
 *
 * Display the maximum RPM value seen so far
 */
void showMaxRPM() {
  clearLCD = false;
  lcd.clear();
  lcd.print(F("MAXIMUM RPM"));
  lcd.setCursor(0, 1);
  lcd.print(maxRPM, DEC);  // Display the maximum recorded RPM
  lcd.print(F("   RPM"));
  delay(PAGE_LENGTH);

  lcd.clear();
  lcd.print(F("IDLE STATE"));
  lcd.setCursor(0, 1);
  lcd.print(F("READY TO MEASURE"));
  // delay(PAGE_LENGTH);
} // ./showMaxRPM
