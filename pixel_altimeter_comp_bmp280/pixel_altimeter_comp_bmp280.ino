// LED Skydive Altimeter by Martin Hovmöller
// To do:
// 1) Detect descent, don't light up LEDs on ascent.
// 2) Deep sleep power mode.
// 3) Make an array to handle the different altitudes.
//#define simulation

#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP280.h>



// BMP280 pins
#define BMP_SCK 6  // SCL (SCK)
#define BMP_MISO 9 // SDO
#define BMP_MOSI 7 // SDA (SDI)
#define BMP_CS 8   // CSB (CS)

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

#define PIN 10
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN);

// Define different colors for easier use.
uint32_t blue      = strip.Color(0, 0, 255);
uint32_t green     = strip.Color(0, 255, 0);
uint32_t red       = strip.Color(255, 0, 0);
uint32_t yellow    = strip.Color(255, 255, 0);
uint32_t off       = strip.Color(0, 0, 0);

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
  Serial.begin(9600);

  /*if (bmp.begin())
    Serial.println("BMP180 init success");
    else {
    Serial.println("BMP180 init fail (disconnected?)\n\n");
    while(1);
    }*/

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
  agl = agl - 2;
  delay(20);
#else
  agl = bmp.readAltitude() - read_baseline.field1;
#endif

  // Light up or blink the LEDs in different colors depending on altitude.
  // Competition window configuration.
  if (agl > 3300) {
    setLEDcolor(off);
  }

  else if (agl >= 3200) {
    setLEDcolor(green);
  }
  else if (agl >= 3100) {
    setLEDcolor(blue);
  }
  else if (agl >= 3000) {
    setLEDcolor(red);
  }
  else if (agl >= 2300) {
    setLEDcolor(off);
  }
  else if (agl >= 2200) {
    setLEDcolor(green);
  }
  else if (agl >= 2100) {
    setLEDcolor(red);
  }
  else if (agl >= 2000) {
    setLEDcolor(off);
  }
  else {
    setLEDcolor(off);
  }
}
