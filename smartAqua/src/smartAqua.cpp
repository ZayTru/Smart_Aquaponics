/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"

const int  pumpPin = D16;
const int servoPin = D14;
const int fanPin = D15;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int THERMISTOR_PIN = A2;

//Fan Thresholds======================================================================================================================
const float FAN_ON_TEMP = 80.0;
const float FAN_OFF_TEMP = 78.0;
const float FAN_ON_HUMIDITY = 65.0;
const float FAN_OFF_HUMIDITY = 62.0;
bool isFanRunning = false;

//thermister constants=================================================================================================================
const float SERIES_RESISTOR = 10000.0;
const float NOMINAL_RESISTANCE = 10000.0;
const float NOMINAL_TEMPERATURE = 25.0;
const float B_COEFFICIENT = 3950.0;
//schedule time=========================================================================================================================
const int MORNING_HOUR = 8;
const int EVENING_HOUR = 16;
const float TIMEZONE_OFFSET = -7.0;

int lastSpinMinute = -1;

//pump timer settigs===================================================================================================================
const unsigned long PUMP_ON_DURATION = 60000;
const unsigned long PUMP_OFF_DURATION = 180000;

unsigned long previousPumpMillis = 0;
bool isPumpRunning = true;

const int OLED_RESET = -1;

//Two OLED Displays====================================================================================================================
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_SSD1306 display2(OLED_RESET);

//BME280 Sensor========================================================================================================================
Adafruit_BME280 bme;

Servo myServo;

SYSTEM_MODE(AUTOMATIC);

void spinServo();
void resetDisplays();


void setup() {
  Serial.begin(9600);

  Time.zone(TIMEZONE_OFFSET);

//initialize First OLED====================================================================================================================
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(0);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();

  //intitalize Second OLED==================================================================================================================
  display2.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display2.setRotation(0);
  display2.clearDisplay();
  display2.setTextColor(WHITE);
  display2.display();

  //initialize BME280========================================================================================================================
  if(!bme.begin(0x76)) {
    if(!bme.begin(0x77)) {
      Serial.print("could not find a valid bme sensor!");
    }
  }

  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, HIGH);
  previousPumpMillis = millis();

  //Initialize Fan pin=====================================================================================================================
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
  
  myServo.attach(servoPin);
  myServo.write(0);
 
}

void loop() {
  //Read Water Thermistor=================================================================================================================
  int analogValue = analogRead(THERMISTOR_PIN);
  float currentResistance = SERIES_RESISTOR * (4095.0 / analogValue-1.0);
  float temperatureKelvin;
  temperatureKelvin = currentResistance / NOMINAL_RESISTANCE;
  temperatureKelvin = log(temperatureKelvin);
  temperatureKelvin /= B_COEFFICIENT;
  temperatureKelvin += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
  temperatureKelvin = 1.0 / temperatureKelvin;

  float temperatureCelsius = temperatureKelvin - 273.15;
  float temperatureFahrenheit = (temperatureCelsius * 9.0 / 5.0) +32.0;

  //update display========================================================================================================================
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.printf("WATER TEMPERATURE");

  display.setTextSize(2);
  display.setCursor(0, 22);
  display.print(temperatureFahrenheit, 1);
  display.print(" F");

  display.display();
  
  // Read BME280 sensor=============================================================================================================
  float bmeTempC = bme.readTemperature();
  float bmeTempF = (bmeTempC * 9.0 / 5.0)+ 32.0;
  float bmeHumidity = bme.readHumidity();

  //Fan Control========================================================================================================================
  if(!isFanRunning) {
    if(bmeTempF > FAN_ON_TEMP || bmeHumidity > FAN_ON_HUMIDITY) {
      digitalWrite(fanPin, HIGH);
      isFanRunning = true;
    }
  } else {
    if(bmeTempF < FAN_OFF_TEMP && bmeHumidity < FAN_OFF_HUMIDITY) {
      digitalWrite(fanPin, LOW);
      isFanRunning = false;

      display.setRotation(0);
      display2.setRotation(0);
    }
  }

  //Display 2
  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setCursor(0,0);
  display2.print("DOME TEMP:");
  if(isFanRunning) {
    display2.setCursor(80,0);
    display2.print("[FAN]");
  }

  display2.setTextSize(2);
  display2.setCursor(0,11);
  display2.print(bmeTempF,1);
  display2.print(" F");

  display2.setTextSize(1);
  display2.setCursor(0,34);
  display2.print("DOME HUMIDITY:");

  display2.setTextSize(2);
  display2.setCursor(0,45);
  display2.print(bmeHumidity, 1);
  display2.print(" %");
  display2.display();

  delay(500);

  int currentMinute = Time.minute();
  if(currentMinute != lastSpinMinute) {
    spinServo();
    lastSpinMinute = currentMinute;
  }
  
   unsigned long currentMillis = millis();

   if(isPumpRunning) {
    if (currentMillis - previousPumpMillis >= PUMP_ON_DURATION) {
      digitalWrite(pumpPin, LOW);
      isPumpRunning = false;
      previousPumpMillis = currentMillis;
      resetDisplays();

    }
   } else {
    if (currentMillis - previousPumpMillis >= PUMP_OFF_DURATION) {
      digitalWrite(pumpPin, HIGH);
      isPumpRunning = true;
      previousPumpMillis = currentMillis;
      resetDisplays();
   }
 }
}
void spinServo() {
  myServo.write(180);
  delay(500);
  myServo.write(0);
  delay(500);
}
void resetDisplays() {
    delay(100);
      display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      display.setRotation(0);
      display.setTextColor(WHITE);

      display2.begin(SSD1306_SWITCHCAPVCC, 0x3D);
      display2.setRotation(0);
      display2.setTextColor(WHITE);
  
}