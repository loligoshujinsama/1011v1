// This library is used for communication with I2C devices, such as the BME680 board
#include <Wire.h>

// This library is used to print sensor values to a TinyScreen
#include <TinyScreen.h>
#include "TinyAnimation.h"

// These libraries are used to interface with the BME680 Sensor
#include "Adafruit_Sensor.h"
#include "Adafruit_BME680.h"

// Global Sensor Variables
#define SEALEVELPRESSURE_HPA (1013.25) // used to find approximate altitude 
Adafruit_BME680 bme; // I2C

// TinyScreen Global Variables
TinyScreen display = TinyScreen(TinyScreenDefault);
int background = TS_8b_Black; // sets the background color to black
PercentBarHorizontal terriblebar(display, 2, 2, 94, 28, TS_8b_Gray, TS_8b_DarkRed, TS_8b_White, 0, "poor", "rich", true);
PercentBarHorizontal poorbar(display, 2, 2, 94, 28, TS_8b_Gray, TS_8b_Red, TS_8b_White, 0, "poor", "rich", true);
PercentBarHorizontal averagebar(display, 2, 2, 94, 28, TS_8b_Gray, TS_8b_Yellow, TS_8b_White, 0, "poor", "rich", true);
PercentBarHorizontal goodbar(display, 2, 2, 94, 28, TS_8b_Gray, TS_8b_Green, TS_8b_White, 0, "poor", "rich", true);
PercentBarHorizontal excellentbar(display, 2, 2, 94, 28, TS_8b_Gray, TS_8b_DarkGreen, TS_8b_White, 0, "poor", "rich", true);

// Used to control how often sensor values are updated in the main loop()
unsigned long delayTime = 20;

// The power pin for our board, used for digitally writing to output
const int powerPin = 4;
const bool airQualityGraphic = true; // if set to true air quality graphic will be displayed instead of text output of all sensor data
const bool raw = false; // if set to true, raw VOC data will be displayed instead of an airQuality percentage based on that data
const bool displayAlt = false; // if set to true, altitude will be displayed on the TinyScreen+ instead of air quality/raw due to shortage of room. Will not impact serial output.
float airQuality = 0.00;
float allTimeHighest = 0.00;
float highestInRange = 0.00;
float secondHighestInRange = 0.00;
unsigned long highRangeTime = millis();
unsigned long secondRangeTime = millis();
const int MAX = 550000; // theoretical max raw air quality reading. used for a small percentage of the air quality calculation
const int RANGE_DURATION = 18; // 50% of the VOC portion of the air quality reading will be based on the difference between the current value and the extremes of the last RANGE_DURATION hours

void setup() {
  SerialUSB.begin(9600); // Bandwidth for our communication
  // Print to Serial Monitor
  // You can pass flash-memory based strings to SerialUSB.print() by wrapping them with F().
  // This means you're using flash memory instead of RAM to print stuff
//  while (!SerialUSB);
  SerialUSB.println(F("BME680 test"));

  // We want to see Digital Output from the sensor
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);

  Wire.begin();
  selectPort(0); // The adapter board has 4 different ports (0-3),
  // make sure your software matches the setup!

  // This is the setup used to initialize the TinyScreen's appearance
  display.begin();
  display.setBrightness(15);
  display.setFlip(true);
  display.setFont(thinPixel7_10ptFontInfo);
  display.fontColor(TS_8b_White, background);

  // Set the cursor to the following coordinates before it prints "BME680 Test"
  display.setCursor(7, 54);
  display.print("Fengshui Detector");

  // If the bme sensor is not found, throw statement and stop program
  // If you end up here, check to make sure your value in selectPort() is correct!
  if (!bme.begin(0x76)) {
    display.setCursor(12, 12);
    display.print("No Sensor!");  // Printed to TinyScreen
    SerialUSB.println("Could not find a valid BME680 sensor, check wiring!"); // Printed to Serial Monitor
    while (1); // loop forever, because the rest of the program means nothing without the sensor
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  terriblebar.updateBarEndInfo(); // Apply the letters to the screen once so we don't have to every tick
}

// Forever looping the following logic
void loop() {
  if (! bme.performReading()) {
    SerialUSB.println("Failed to perform reading :(");
    return;
  }
  validateRange();
  compareAirQuality();
  calculateQuality();

  airQuality /= 100;
  if(airQuality*100 < 30) // terrible
  {
     terriblebar.tick(airQuality);
  }
  else if(airQuality*100 < 50  && airQuality*100 >=30) // poor
  {
     poorbar.tick(airQuality);
  }
  else if(airQuality*100 < 70 && airQuality*100 >=50) // average
  {
     averagebar.tick(airQuality);
  }
 else if(airQuality*100 < 80 && airQuality*100 >=70) // good
  {
     goodbar.tick(airQuality);
  }
 else //(airQuality*100 >=80) excellent
  {
     excellentbar.tick(airQuality);
  }
  airQuality *= 100;
  delay(delayTime); // How often values are updated and printed  
}

// **This function is necessary for all Whisker boards attached through an Adapter board**
// Selects the correct address of the port being used in the Adapter board
void selectPort(int port) {
  Wire.beginTransmission(0x70); //I2C
  Wire.write(0x04 + port);
  Wire.endTransmission();
}

void compareAirQuality()
{
  float temp = (bme.gas_resistance/1000);
  if(temp>allTimeHighest)
  {
    allTimeHighest = temp;    
  }
   if(temp>highestInRange)
  {
    secondHighestInRange = highestInRange; 
    secondRangeTime = highRangeTime; 
    highestInRange = temp;  
    highRangeTime = millis();
  }
}

void validateRange()
{
  if(((((millis()-highRangeTime)/1000)/60)/60)>RANGE_DURATION)
  {
    highestInRange = secondHighestInRange;
  }
  if(((((millis()-secondRangeTime)/1000)/60)/60)>RANGE_DURATION)
  {
    secondHighestInRange = bme.gas_resistance/1000;
  }
}

void calculateQuality()
{
  float raw = (bme.gas_resistance/1000);
  float rawHumidity = bme.readHumidity();
  float shortTermQuality = 1-((highestInRange-raw)/highestInRange);
  float longTermQuality = 1-((allTimeHighest-raw)/allTimeHighest);
  float theoreticalQuality = 1-((MAX-raw)/MAX);
  float humidityIndex = 1-(abs(rawHumidity-40.00)/100); // absolute difference between current humidity and optimal humidity
  airQuality = (0.75*((shortTermQuality*0.5)+(longTermQuality*0.25)+(theoreticalQuality*0.25))+(humidityIndex*0.25))*100.00;
  SerialUSB.print("airQuality: ");
  SerialUSB.print(airQuality);
  SerialUSB.println("%");
}
