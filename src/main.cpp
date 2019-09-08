#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>

const char SSID[] = "SSID";
const char SSPW[] = "SSPW";

const String SENSOR_NAME = "ESP8266";
int lastValue = LOW;

WiFiClient net;
MQTTClient mqtt;

void messageReceived(String &topic, String &payload) {
  Serial.println("Received: " + topic + ":");
  Serial.println(payload);
  Serial.print("----\n");
}

void mqttConnect() {
  mqtt.begin("192.168.0.129", net);
  mqtt.onMessage(messageReceived);
  while (!mqtt.connect("esp8266", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("MQTT client connected.");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting device...");

  // Connect to network
  WiFi.begin(SSID, SSPW);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  // Connect to mqtt
  mqttConnect();

  Serial.println("Setting up inputs...");
  pinMode(D0, INPUT);
  delay(2000);
}

void setSensorValue(int newValue) {
  if (newValue != lastValue) {
    if (newValue == HIGH) {
      Serial.println("Magnetic sensor closed.");
    } else {
      Serial.println("Magnetic sensor open!");
    }

    char payload[100];
    sprintf(payload, "{ \"newStatus\": %d, \"oldStatus\": %d }", newValue, lastValue);

    mqtt.publish("sensor/" + SENSOR_NAME + "/reed", payload);
    lastValue = newValue;
  }
}

void mqttLoop() {
  mqtt.loop();
  if (!mqtt.connected()) {
    Serial.println("MQTT disconnected!");
    mqttConnect();
  }
}

void loop() {
  mqttLoop();

  int val = digitalRead(D0);
  setSensorValue(val);

  delay(500);
}