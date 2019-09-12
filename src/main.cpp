#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <WiFiManager.h>
#include "sensor.h"


const char* MQTT_HOST = "192.168.0.10";
const String DEPLOYMENT_NAME = "eggleo";

WiFiClient net;
MQTTClient mqtt;
WiFiManager wifiManager;

const int NO_SENSORS = 2;
Sensor sensors[NO_SENSORS] = {
  Sensor("livingroom-right", D1, D2),
  Sensor("livingroom-left", D5, D6)
};

void messageReceived(String &topic, String &payload) {
  Serial.println("Received: " + topic + ":");
  Serial.println(payload);
  Serial.print("----\n");
}

void mqttConnect() {
  Serial.printf("Attempting to connect to %s\n", MQTT_HOST);
  mqtt.begin(MQTT_HOST, net);
  mqtt.onMessage(messageReceived);
  while (!mqtt.connect(DEPLOYMENT_NAME.c_str(), "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("MQTT client connected.");
}

void notifyMqtt(String sensorName, int lastValue, int newValue, int count) {
    char payload[100];
    sprintf(payload, "{ \"status\": %d, \"prevStatus\": %d, \"changes\": %d, millis: %d }", newValue, lastValue, count, (int)millis());

    mqtt.publish("sensor/" + DEPLOYMENT_NAME + "/" + sensorName, payload);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("Booting device...");

  wifiManager.autoConnect();

  // Connect to mqtt
  mqttConnect();

  Serial.println("Setting up inputs...");
  for (int i = 0; i < NO_SENSORS; i++) {
    sensors[i].setup();
    sensors[i].registerChangeCallback(notifyMqtt);
  }

  delay(2000);
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

  for (int i = 0; i < NO_SENSORS; i++) {
    sensors[i].readValue();
  }

  delay(500);
}