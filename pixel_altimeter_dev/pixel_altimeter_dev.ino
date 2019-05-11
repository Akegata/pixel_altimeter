// LED Skydive Altimeter by Martin Hovmöller
// To do:
// 1) Detect descent, don't light up LEDs on ascent.
// 2) Deep sleep power mode.
// 3) Make an array to handle the different altitudes.

//#define BMP280 // Use BMP280. If not defined, BMP180 will be used instead.
#define simulation

#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

//Adafruit_BMP280 bme; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
#ifdef BMP280
  #include <Adafruit_BMP280.h>  

  // BMP280 pins
  #define BMP_SCK 6  // SCL (SCK)
  #define BMP_MISO 9 // SDO
  #define BMP_MOSI 7 // SDA (SDI)
  #define BMP_CS 8   // CSB (CS)

  Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
#else
  #include <Adafruit_BMP085.h>
  Adafruit_BMP085 bmp;
#endif


#define PIN 10
//#define PIN 2
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
int i = 0;
void setup() {


  Serial.begin(9600);  


   
  lcd.init(); 

  while (i < 200) {
 digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(300);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(300); 
  i++   ;
}
  lcd.backlight();

  lcd.clear();
  if (bmp.begin())
  Serial.println("Barometer init success");
  else {
    Serial.println("Barometer init fail (disconnected?)\n\n");
    while(1);
  }


  //<bmp.begin();
  strip.begin();
  strip.show();

     lcd.print("Setting LED to blue");

    setLEDcolor(blue);
      delay(5000);
     lcd.print("LED set to blue");


  powercycles = (EEPROM.read(powercycles_address));
  lcd.setCursor(0, 0);
  lcd.print("Powercycles: ");
  lcd.print(powercycles);
  
  // On the third power cycle, reset the calibration
  if (powercycles == 2) {
      lcd.setCursor(0, 0);
  lcd.print("Altitude: ");
  lcd.print(bmp.readAltitude());
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
                   // wait for a second

  #ifdef simulation
    agl = agl - 100;
    delay(500);
  #else
    agl = bmp.readAltitude() - read_baseline.field1;
  #endif

    lcd.clear();
    lcd.setCursor(0, 0);

        //read altitude
    lcd.print("Alt diff: ");
    lcd.print(agl);
    lcd.print(" m");
    lcd.setCursor(0, 1);
  lcd.print("B: ");
  lcd.print(round(read_baseline.field1));
  lcd.print(" A: ");
  lcd.print(round(bmp.readAltitude()));
  

/* lcd.print("T:");
  lcd.print(round(bmp.readTemperature()));
  lcd.print(" P: ");
  lcd.print(bmp.readPressure()); */
  
    
    Serial.print("Temperature = ");
    Serial.print(round(bmp.readTemperature()));
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print("Approx altitude = ");
    Serial.print(bmp.readAltitude()); // this should be adjusted to your local forcase
    Serial.println(" m");
    
    Serial.println();
//    delay(2000);

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
