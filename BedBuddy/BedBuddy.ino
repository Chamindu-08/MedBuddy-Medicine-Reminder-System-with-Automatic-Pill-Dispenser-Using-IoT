#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>

#define _SSID "gift"
#define _PASSWORD ""
#define REFERENCE_URL "https://medbuddy-85345-default-rtdb.firebaseio.com/"

Firebase firebase(REFERENCE_URL);

void setup() {
  Serial.begin(9600);

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
    Serial.println(firebase.error());
  }
}

void loop() {

}
