#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

// --------------------
// WiFi (required for Wokwi)
// --------------------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// --------------------
// MQTT
// --------------------
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// Use unique topics so other students/users on the public broker
// are less likely to interfere with the simulation.
const char* LED_TOPIC  = "ENGR4399/UrielGarcia/LED";
const char* TEMP_TOPIC = "ENGR4399/UrielGarcia/TEMP";

// --------------------
// ESP32 pins
// --------------------
const int THERMISTOR_PIN = 34;  // ADC1 pin - works while WiFi is active
const int BUTTON_PIN = 27;
const int LED_PIN = 26;

// Wokwi NTC sensor defaults
const float BETA = 3950.0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool lastButtonState = LOW;
unsigned long lastReconnectAttempt = 0;

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

float readTemperatureC() {
  int analogValue = analogRead(THERMISTOR_PIN);

  // Wokwi NTC module: 10k NTC + 10k resistor.
  // ESP32 ADC is 12-bit: 0..4095.
  if (analogValue <= 0 || analogValue >= 4095) {
    return NAN;
  }

  float celsius =
      1.0 / (log(1.0 / (4095.0 / analogValue - 1.0)) / BETA
      + 1.0 / 298.15) - 273.15;

  return celsius;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("MQTT received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Assignment requirement: demonstrate JSON parsing.
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    Serial.println("Expected example: {\"led\":\"on\"}");
    return;
  }

  if (String(topic) == LED_TOPIC) {
    const char* state = doc["led"];

    if (state == nullptr) {
      Serial.println("JSON is missing the 'led' field.");
      return;
    }

    if (strcmp(state, "on") == 0) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    } else if (strcmp(state, "off") == 0) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    } else {
      Serial.println("LED value must be 'on' or 'off'.");
    }
  }
}

bool connectMQTT() {
  Serial.print("Connecting to MQTT broker... ");

  String clientId = "ENGR4399-Uriel-ESP32-";
  clientId += String(random(0xffff), HEX);

  if (mqttClient.connect(clientId.c_str())) {
    Serial.println("connected!");
    mqttClient.subscribe(LED_TOPIC);

    Serial.print("Subscribed to: ");
    Serial.println(LED_TOPIC);
    Serial.print("Temperature publishes to: ");
    Serial.println(TEMP_TOPIC);
    return true;
  }

  Serial.print("failed, state=");
  Serial.println(mqttClient.state());
  return false;
}

void publishTemperature() {
  float temperatureC = readTemperatureC();

  if (isnan(temperatureC)) {
    Serial.println("Temperature reading failed.");
    return;
  }

  // Assignment requirement: demonstrate JSON construction.
  JsonDocument doc;
  doc["temperature"] = round(temperatureC * 100.0) / 100.0;
  doc["unit"] = "C";
  doc["device"] = "ESP32";
  doc["source"] = "NTC";

  char jsonBuffer[160];
  serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

  bool ok = mqttClient.publish(TEMP_TOPIC, jsonBuffer);

  Serial.print(ok ? "Published JSON: " : "Publish failed: ");
  Serial.println(jsonBuffer);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  digitalWrite(LED_PIN, LOW);

  analogReadResolution(12);

  randomSeed(micros());

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);

  connectMQTT();

  Serial.println();
  Serial.println("=== ENGR 4399 MQTT Simulation Ready ===");
  Serial.println("Press the green button to publish temperature JSON.");
  Serial.println("Send {\"led\":\"on\"} to the LED topic to turn LED on.");
  Serial.println("Send {\"led\":\"off\"} to the LED topic to turn LED off.");
  Serial.println();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= 3000) {
      lastReconnectAttempt = now;
      connectMQTT();
    }
  } else {
    mqttClient.loop();
  }

  bool buttonState = digitalRead(BUTTON_PIN);

  // Publish once on each button press
  if (buttonState == HIGH && lastButtonState == LOW) {
    delay(30); // simple debounce
    if (digitalRead(BUTTON_PIN) == HIGH) {
      Serial.println("Button pressed.");
      publishTemperature();
    }
  }

  lastButtonState = buttonState;
  delay(10);
}
