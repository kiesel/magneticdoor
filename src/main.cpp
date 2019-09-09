#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <WiFiManager.h>
#include "sensor.h"


const char* MQTT_HOST = "192.168.0.129";
const String DEPLOYMENT_NAME = "prototype";

WiFiClient net;
MQTTClient mqtt;
WiFiManager wifiManager;

Sensor sensor = Sensor("door1", D1, D2);

void messageReceived(String &topic, String &payload) {
  Serial.println("Received: " + topic + ":");
  Serial.println(payload);
  Serial.print("----\n");
}

void mqttConnect() {
  Serial.printf("Attempting to connect to %s\n", MQTT_HOST);
  mqtt.begin(MQTT_HOST, net);
  mqtt.onMessage(messageReceived);
  while (!mqtt.connect("esp8266", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("MQTT client connected.");
}

void notifyMqtt(String sensorName, int lastValue, int newValue, int count) {
    char payload[100];
    sprintf(payload, "{ \"status\": %d, \"oldStatus\": %d, \"changes\": %d, millis: %d }", newValue, lastValue, count, (int)millis());

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
  sensor.setup();
  sensor.registerChangeCallback(notifyMqtt);

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

  sensor.readValue();

  delay(500);
}