#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <sensor.h>

const char SSID[] = "SSID";
const char SSPW[] = "SSPW";
const String DEPLOYMENT_NAME = "prototype";

WiFiClient net;
MQTTClient mqtt;

Sensor sensor = Sensor("door1", D1, D2);

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

void notifyMqtt(String sensorName, int lastValue, int newValue, int count) {
    char payload[100];
    sprintf(payload, "{ \"status\": %d, \"oldStatus\": %d, \"changes\": %d, millis: %d }", newValue, lastValue, count, millis());

    mqtt.publish("sensor/" + DEPLOYMENT_NAME + "/" + sensorName, payload);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting device...");

  // Connect to network
  if (WiFi.SSID() != SSID) {
    WiFi.begin(SSID, SSPW);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
  }

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