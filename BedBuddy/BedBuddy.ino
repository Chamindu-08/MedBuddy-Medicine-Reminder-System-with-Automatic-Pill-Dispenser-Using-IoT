#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ArduinoJson.h>

#define _SSID "gift"
#define _PASSWORD ""
#define REFERENCE_URL "https://medbuddy-85345-default-rtdb.firebaseio.com/"

Firebase firebase(REFERENCE_URL);
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//servo motor setup
Servo servoA, servoB, servoC;
const int servoPinA = 12, servoPinB = 13, servoPinC = 15;
const int startAngle = 0;

//buzzer and button
const int buzzerPin = 14;
const int buttonPin = 2;

  //track last button press time
unsigned long lastButtonPress = 0;
const unsigned long BUTTON_DELAY = 60000;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  //LCD initialization
  lcd.begin();
  lcd.backlight();
  lcd.print("    MedBuddy");
  delay(1000);

  //connect to wifi
  Serial.println("Connecting to: " + String(_SSID));
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, HIGH);

  //firebase test connection
  firebase.setString("/test_connection", "Connected");
  Serial.println("Firebase Connected Successfully");

  //initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
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

  //initialize buzzer and button
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  getScheduleFromFirebase();

  //check if button is pressed and handle delay
  if (digitalRead(buttonPin) == LOW) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastButtonPress >= BUTTON_DELAY) {
      sendEmergencyAlert();
      lastButtonPress = currentMillis;
    } else {
      Serial.println("Button pressed too soon, ignoring...");
    }
    delay(500);
  }
}

void getScheduleFromFirebase() {
  String userId = "userId1";
  DateTime now = rtc.now();
  String currentTime = String(now.hour()) + ":" + String(now.minute());

  //fetch schedule as JSON
  String jsonData = firebase.getString("/users/" + userId + "/schedules");
  if (jsonData == "") {
    Serial.println("Error fetching schedule data");
    return;
  }

  Serial.println("Fetched JSON Data: " + jsonData);

  //parse JSON
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  if (error) {
    Serial.println("JSON Parsing Failed!");
    return;
  }

  //iterate through schedules
  for (JsonPair schedule : doc.as<JsonObject>()) {
    String scheduleId = schedule.key().c_str();
    JsonObject scheduleData = schedule.value().as<JsonObject>();

    String scheduleTime = scheduleData["time"].as<String>();
    String status = scheduleData["status"].as<String>();

    if (scheduleTime == currentTime && status == "pending") {
      Serial.println("Time to dispense pills!");

      int pillA = scheduleData["pillA"];
      int pillB = scheduleData["pillB"];
      int pillC = scheduleData["pillC"];

      if (pillA > 0) dispensePills(servoA, pillA);
      if (pillB > 0) dispensePills(servoB, pillB);
      if (pillC > 0) dispensePills(servoC, pillC);

      buzzForOneMinute();
      updateStatus(userId, scheduleId);
    }
  }
}

void dispensePills(Servo &servo, int pills) {
  for (int i = 0; i < pills; i++) {
    servo.write(180);
    delay(500);
    servo.write(0);
    delay(500);
  }
}

void buzzForOneMinute() {
  unsigned long startTime = millis();
  while (millis() - startTime < 60000) {
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);
    delay(1000);
  }
}

void updateStatus(String userId, String scheduleId) {
  firebase.setString("/users/" + userId + "/schedules/" + scheduleId + "/status", "taken");
  firebase.setInt("/users/" + userId + "/schedules/" + scheduleId + "/updatedAt", millis());
  Serial.println("Status updated to 'taken'");
}

void sendEmergencyAlert() {
  Serial.println("Emergency Button Pressed! Sending alert...");

  String path = "/users/userId1/emergencyAlert/" + String(millis());

  firebase.setString(path + "/message", "Emergency Button Pressed!");
  firebase.setInt(path + "/timestamp", millis());

  Serial.println("Alert Sent!");
}