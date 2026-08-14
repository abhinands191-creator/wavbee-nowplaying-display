#include <WiFi.h>

const char* ssid = "LAPTOP-GAM2I2F1 5416";
const char* password = "11111111";

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("WiFi event: ");
  Serial.println(event);

  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    Serial.print("Disconnect reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Connection failed. WiFi status: ");
    Serial.println(WiFi.status());
  }
}

void loop() {
}