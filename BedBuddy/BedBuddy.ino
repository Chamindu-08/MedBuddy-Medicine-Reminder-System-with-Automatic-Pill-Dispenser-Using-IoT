#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define _SSID "gift"
#define _PASSWORD ""
#define REFERENCE_URL "https://medbuddy-85345-default-rtdb.firebaseio.com/"

Firebase firebase(REFERENCE_URL);
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//servo motor setup
Servo servoA;
Servo servoB;
Servo servoC;
const int servoPinA = 12;  // D6 -> GPIO 12
const int servoPinB = 13;  // D7 -> GPIO 13
const int servoPinC = 15;  // D8 -> GPIO 15
const int startAngle = 0;

void setup() {
  Serial.begin(9600);

  //LCD initialization
  lcd.begin();
  lcd.backlight();
  lcd.print("    MedBuddy");
  delay(1000);

  //connect to the wifi
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  //test the wifi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("\nWiFi Connected");

  //test the firebase connection
  if (firebase.setString("test_connection", "Connected")) {
    Serial.println("Firebase Connected Successfully");
  } else {
    Serial.println("Firebase Connection Failed!");
  }

  //initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  //check if RTC lost power and set time
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //servo motor initialization
  servoA.attach(servoPinA);
  servoB.attach(servoPinB);
  servoC.attach(servoPinC);
  servoA.write(startAngle);
  servoB.write(startAngle);
  servoC.write(startAngle);
}

void loop() {

}
