// LED Skydive Altimeter by Martin Hovmöller
// To do:
// 1) Detect descent, don't light up LEDs on ascent.
// 2) Deep sleep power mode.
// 3) Make an array to handle the different altitudes.
//#define simulation

#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

#define PIN 2
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN);

// Define different colors for easier use.
uint32_t blue      = strip.Color(0, 0, 255);
uint32_t green     = strip.Color(0, 255, 0);
uint32_t red       = strip.Color(255, 0, 0);
uint32_t yellow    = strip.Color(255, 255, 0);
uint32_t off       = strip.Color(0, 0, 0);
//

// Set up structure for alarm states.
#define ONGROUND     1
#define ARMED        2
#define ONALTITUDE   4
#define FREEFALL     8
#define UNDERCANOPY  16
#define LANDED       32
#define NUMBER_OF_ALARMS 6
#define smoothness 0.35f
#define numberOfSamplesForCalibration  5

int state = ONGROUND;
bool onAltitude = false;

float GroundLevelPressure = 1023.35;
int prevTime = 0;
float prevAltitude = 0.0f;


void freefallAlarm1();
void freefallAlarm2();
void freefallAlarm3();
void canopyAlarm1();
void canopyAlarm2();
void canopyAlarm3();


typedef void (*alarmPtr)();

typedef struct Alarm
{
  bool triggered;
  float Speed;
  float Altitude;
  alarmPtr alarm;
}_Alarm;

Alarm Alarms[NUMBER_OF_ALARMS]
{
  {false, 30, 1525, &freefallAlarm1 },
  {false, 30, 1100, &freefallAlarm2 },
  {false, 30,  700, &freefallAlarm3 },
  {false,  0,  300, &canopyAlarm1 },
  {false,  0,  200, &canopyAlarm2 },
  {false,  0,  100, &canopyAlarm3 },
};

#ifdef simulation
  int agl                 = 4000; // Set this for simulating a jump
#else
  int agl                 = 0;
#endif
int baseline_address    = 1; // Address in the EEPROM where the baseline reading should be stored.
int powercycles_address = 0;
int startup             = 0;
int powercycles;
double baseline;

struct eeprom_entry {
  double field1;
  byte field2;
  char name[10];
};
eeprom_entry read_baseline;

// Sets the specified numbers of LEDs to the specifiedcolor.
int setLEDcolor(uint32_t color) {
  strip.setPixelColor(0, color);
  strip.show();
}

// Blinks the specified number of LED's at the specified interval, with the specified color.
int blinkLEDcolor(uint32_t color, int on_time, int off_time) {
  strip.setPixelColor(0, color);
  strip.show();
  delay(on_time);
  strip.setPixelColor(0, off);
  strip.show();
  delay(off_time);
}

void setup() {
  bmp.begin();
  strip.begin();
  strip.show();

  powercycles = (EEPROM.read(powercycles_address));

  // On the third power cycle, reset the calibration
  if (powercycles == 2) {
    baseline = bmp.readAltitude();
    EEPROM.put(baseline_address, baseline);

    blinkLEDcolor(red, 500, 500);
  }
  // Update the powercycle count.
  else {
    powercycles++;
    EEPROM.write(powercycles_address, powercycles);
    blinkLEDcolor(blue, 200, 200);
    delay(1000);
  }

  // Reset the power cycle count and read the baseline from EEPROM.
  powercycles = (0);
  EEPROM.write(0, powercycles);
  EEPROM.get(baseline_address, read_baseline);

  // Blink LED's green to indicate that the altimeter is running.
  blinkLEDcolor(green, 100, 100);
}

void loop() {
  #ifdef simulation
    agl = agl - 100;
    delay(500);
  #else
    agl = bmp.readAltitude() - read_baseline.field1;
  #endif

  int currTime = millis();
  //Calculate deltaTime, since the previous instance
  int dt = currTime - prevTime;
  float altitude = getAltitude();
  float currSpeed = (prevAltitude - altitude) / ((float)(dt) / 1000.0f);
  prevAltitude = altitude;
  prevTime = currTime;

  if (state == ONGROUND && altitude > 300)
  {
    state = ARMED;
    armedAlarm();
  }
  if (state == ARMED && !onAltitude && altitude > 3500 )
  {
    atAltitudeAlarm();
    onAltitude = true;
  }
  else if (state == ARMED && currSpeed > 30)
  {
    state = FREEFALL;
    beep(1000, 3, 50, 300);
  }
  else if (state == FREEFALL && currSpeed < 20)
  {
    state = UNDERCANOPY;
  }
  else if (state == UNDERCANOPY && altitude < 8)
  {
    state = LANDED;
  }


  // Light up or blink the LEDs in different colors depending on altitude.
  if (agl > 3500) {
    setLEDcolor(blue);
  }
  else if (agl >= 3000) {
    blinkLEDcolor(blue, 800, 800);
  }
  else if (agl >= 2500) {
    setLEDcolor(green);
  }
  else if (agl >= 2000) {
    blinkLEDcolor(green, 800, 800);
  }
  else if (agl >= 1500) {
    setLEDcolor(yellow);
  }
  else if (agl >= 1000) {
    blinkLEDcolor(red, 800, 800);
  }
  else if (agl >= 700) {
    setLEDcolor(red);
  }
  else {
    setLEDcolor(off);
  } 
}
