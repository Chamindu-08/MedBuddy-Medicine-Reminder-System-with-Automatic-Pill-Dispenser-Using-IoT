#include <ESP8266WiFi.h>
#include <ESP8266Firebase.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ArduinoJson.h>

#define WIFI_SSID "gift"
#define WIFI_PASSWORD ""
#define REFERENCE_URL "https://medbuddy-85345-default-rtdb.firebaseio.com/"

Firebase firebase(REFERENCE_URL);
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//servo motors
Servo servoA;
Servo servoB; 
Servo servoC;
const int servoPinA = 12, servoPinB = 13, servoPinC = 15;
const int startAngle = 0;

// Buzzer and button
const int buzzerPin = 14;
const int buttonPin = 2;

unsigned long lastButtonPress = 0;
const unsigned long BUTTON_DELAY = 60000;

//device ID
String deviceID = "GAHDSE241-004";

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Wire.begin(4, 5);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //RTC setup
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, resetting time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("RTC connected");

  //check WIFI connection
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println("WIFI connected");
  Serial.println(WiFi.localIP());

  //firebase test connection
  if (firebase.setString("/test_connection", "Connected")) {
    Serial.println("Firebase connected");
  } else {
    Serial.println("Firebase Connection Failed!");
  }

  //LCD setup
  lcd.begin();
  lcd.backlight();
  lcd.print("    MedBuddy");
  Serial.println("LCD display connected");

  //servo setup
  servoA.attach(servoPinA);
  servoB.attach(servoPinB);
  servoC.attach(servoPinC);
  Serial.println("servo motors connected");

  servoA.write(startAngle);
  servoB.write(startAngle);
  servoC.write(startAngle);
  Serial.println("servo motors rotated start angle");
  

  //button & buzzer
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("setup buzzer and push button");

  Serial.println("Device ID: " + deviceID);
}

void loop() {
  lcd.clear();
  lcd.print("    MedBuddy");
  lcd.setCursor(0, 1);
  lcd.print("Have a Nice Day");

  //check schedules
  checkSchedule();

  //handle emergency button and check schedules
  if (digitalRead(buttonPin) == HIGH) {
    sendEmergencyAlert();
    delay(1000);
  }
}

void checkSchedule() {
  DateTime now = rtc.now();
  String currentTime = formatTime(now.hour(), now.minute());
  String currentDate = String(now.year()) + "-" + twoDigits(now.month()) + "-" + twoDigits(now.day());

  Serial.println("Current Time: " + currentTime);
  Serial.println("Current Date: " + currentDate);

  //fetch all schedules
  String jsonData = firebase.getString("/devices/" + deviceID + "/schedules");
  if (jsonData == "") {
    Serial.println("Failed to fetch schedule data.");
    return;
  }

  Serial.println("Received JSON: " + jsonData);

  //ensure JSON is wrapped in {}
  if (!jsonData.startsWith("{")) {
    jsonData = "{" + jsonData + "}";
  }

  //parse JSON
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  if (error) {
    Serial.println("JSON Parsing Error");
    return;
  }

  Serial.println("JSON Parsing successful.");
  Serial.println("Parsed JSON Structure:");
  serializeJson(doc, Serial);
  Serial.println();

  if (doc.isNull() || doc.size() == 0) {
    Serial.println("Error: No schedules found in JSON.");
    return;
  }

  bool scheduleFound = false;

  //loop through each schedule in the JSON object
  for (JsonPair schedule : doc.as<JsonObject>()) {
    String scheduleId = schedule.key().c_str();
    JsonObject scheduleData = schedule.value().as<JsonObject>();

    String scheduleTime = scheduleData["time"].as<String>();
    String scheduleDate = scheduleData["date"].as<String>();
    String pillA_dose = scheduleData["pillA_dose"].as<String>();
    String pillB_dose = scheduleData["pillB_dose"].as<String>();
    String pillC_dose = scheduleData["pillC_dose"].as<String>();
    String status = scheduleData["status"].as<String>();

    Serial.println("Checking Schedule ID: " + scheduleId);
    Serial.println("Schedule Time: " + scheduleTime);
    Serial.println("Schedule Date: " + scheduleDate);
    Serial.println("Pill A Dose: " + pillA_dose);
    Serial.println("Pill B Dose: " + pillB_dose);
    Serial.println("Pill C Dose: " + pillC_dose);
    Serial.println("Status: " + status);

    //check if the schedule matches today's date and is pending
    if (scheduleDate == currentDate && scheduleTime == currentTime && status == "pending") {
      Serial.println("Dispensing pills for schedule: " + scheduleId);

      lcd.clear();
      lcd.print("Medicine Time");

      int pillA = scheduleData["pillA_dose"];
      int pillB = scheduleData["pillB_dose"];
      int pillC = scheduleData["pillC_dose"];

      if (pillA > 0) dispensePills(servoA, pillA);
      if (pillB > 0) dispensePills(servoB, pillB);
      if (pillC > 0) dispensePills(servoC, pillC);

      Serial.println("pills dispensing sucessful");

      lcd.clear();
      lcd.print("Pick your medicines");

      triggerBuzzer();
      updateStatus(scheduleId);

      scheduleFound = true;
    }
  }

  if (!scheduleFound) {
    Serial.println("No schedules match the current time.");
  }
}

void dispensePills(Servo &servo, int count) {
  for (int i = 0; i < count; i++) {
    for (int i = 0; i <= 180; i++) {
    servo.write(i);
    delay(15);
    }
    delay(1000);

    for (int i = 180; i >= 0; i--) {
      servo.write(i);
      delay(15);
    }
    delay(1000);
  }
}

void triggerBuzzer() {
  unsigned long startTime = millis();
  while (millis() - startTime < 30000) {
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);
    delay(1000);
  }
}

void updateStatus(String scheduleId) {
  firebase.setString("/devices/" + deviceID + "/schedules/" + scheduleId + "/status", "taken");
  firebase.setInt("/devices/" + deviceID + "/schedules/" + scheduleId + "/updatedAt", millis());
  Serial.println("Updated status to 'taken'");
}

String twoDigits(int number) {
  return number < 10 ? "0" + String(number) : String(number);
}

String formatTime(int hour, int minute) {
  String period = (hour >= 12) ? "PM" : "AM";
  hour = (hour > 12) ? hour - 12 : (hour == 0 ? 12 : hour);
  return String(hour) + ":" + twoDigits(minute) + " " + period;
}

void sendEmergencyAlert() {
  Serial.println("Emergency Alert Triggered!");

  //get current date and time
  DateTime now = rtc.now();
  String date = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());

  //convert time to 12-hour format with AM/PM
  int hour = now.hour();
  String period = (hour >= 12) ? "PM" : "AM";

   //convert 0 or 12 to 12-hour format
  hour = (hour % 12 == 0) ? 12 : hour % 12;
  String time = String(hour) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + " " + period;

  Serial.println("Emergency Alert date : "+date);
  Serial.println("Emergency Alert time : "+time);

  //construct the Firebase path
  String path = "/devices/" + deviceID + "/emergencyAlert/";

  //send data to Firebase
  firebase.setString(path + "/message", "Emergency Button Pressed!");
  firebase.setString(path + "/date", date);
  firebase.setString(path + "/time", time);

  Serial.println("Emergency Alert Sent!");

  lcd.clear();
  lcd.print("Emergency Alert Sent");
}
