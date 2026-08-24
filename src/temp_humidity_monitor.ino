/*
  Temperature & Humidity Monitoring System
  Platform : ESP32 (Arduino core)
  Sensor   : DHT22
  Cloud    : ThingSpeak (HTTP REST API)

  Wiring:
    DHT22 VCC -> 3V3
    DHT22 GND -> GND
    DHT22 DATA -> GPIO4 (with a 10k pull-up resistor to 3V3)

  Notes:
    - Uses ThingSpeak's HTTP write API (works from any ESP32, no external
      MQTT broker required). If you prefer MQTT, ThingSpeak also exposes an
      MQTT broker (see docs/setup.md) — swap the HTTP block for PubSubClient.
    - Threshold-based alerting: this sketch triggers ThingSpeak's built-in
      "React" + email-alert feature by writing to a dedicated alert field
      (see docs/setup.md for configuring the MATLAB Analysis-based trigger).
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ---------- USER CONFIG ----------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";
const char* THINGSPEAK_HOST    = "http://api.thingspeak.com/update";

const float TEMP_HIGH_THRESHOLD = 38.0;  // deg C - triggers alert field
const float TEMP_LOW_THRESHOLD  = 5.0;   // deg C - triggers alert field

const unsigned long UPLOAD_INTERVAL_MS = 20000; // ThingSpeak free tier min. interval is 15s
// ----------------------------------

#define DHTPIN  4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastUploadTime = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (millis() - lastUploadTime >= UPLOAD_INTERVAL_MS) {
    lastUploadTime = millis();

    float humidity    = dht.readHumidity();
    float temperature = dht.readTemperature(); // Celsius

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Failed to read from DHT22 sensor. Retrying next cycle."));
      return;
    }

    int alertFlag = 0;
    if (temperature >= TEMP_HIGH_THRESHOLD || temperature <= TEMP_LOW_THRESHOLD) {
      alertFlag = 1;
      Serial.println(F("*** Threshold breached - alert flag set ***"));
    }

    Serial.print(F("Temp: "));
    Serial.print(temperature);
    Serial.print(F(" C  |  Humidity: "));
    Serial.print(humidity);
    Serial.println(F(" %"));

    sendToThingSpeak(temperature, humidity, alertFlag);
  }
}

void connectWiFi() {
  Serial.print(F("Connecting to WiFi"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(F("."));
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print(F("Connected. IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println(F("WiFi connection failed - will retry in main loop."));
  }
}

void sendToThingSpeak(float temperature, float humidity, int alertFlag) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(THINGSPEAK_HOST) +
               "?api_key=" + THINGSPEAK_API_KEY +
               "&field1=" + String(temperature, 2) +
               "&field2=" + String(humidity, 2) +
               "&field3=" + String(alertFlag);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.print(F("ThingSpeak update sent. Entry ID: "));
    Serial.println(http.getString());
  } else {
    Serial.print(F("ThingSpeak update failed, HTTP code: "));
    Serial.println(httpCode);
  }

  http.end();
}
