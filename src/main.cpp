#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <WiFiManager.h>
#include <RTCVars.h>
#include "sensor.h"

WiFiClient net;
MQTTClient mqtt;
WiFiManager wifiManager;
RTCVars state;

const int SERIAL_SPEED = 115200;
const char* MQTT_HOST = "192.168.0.10";
const String DEPLOYMENT_NAME = "eggleo";
const String SENSOR_NAME = String("ESP") + String(ESP.getChipId());

int reset_counter;

// Configure NO_SENSORS, and an appropriate sensors array:
const int NO_SENSORS = 2;
Sensor sensors[NO_SENSORS] = {
  Sensor("livingroom-right", D1, D2),
  Sensor("livingroom-left", D5, D6)
};

void wifiConnect() {
  if (!WiFi.isConnected()) {
    Serial.println("Connecting to Wifi");
  }

  while (!WiFi.isConnected()) {
    wifiManager.autoConnect();
    Serial.print(".");
    delay(500);
  }
}

void mqttConnect() {
  if (mqtt.connected()) {
    return;
  }

  wifiConnect();

  Serial.printf("Attempting to connect to %s\n", MQTT_HOST);
  mqtt.begin(MQTT_HOST, net);

  while (!mqtt.connected()) {
    mqtt.connect(SENSOR_NAME.c_str(), "try", "try");
    Serial.print(".");
    delay(1000);
  }

  Serial.println("MQTT client connected.");
}

void mqttPublish(char* topic, char* payload) {
  Serial.printf("About to publish MQTT message to topic %s:\n%s\n", topic, payload);
  mqttConnect();

  bool res = mqtt.publish(topic, payload, strlen(payload), false, 1);
  if (!res) {
    Serial.printf("Error when publishing msg: %d\n", mqtt.lastError());
  } else {
    Serial.println("Message published!");
  }
}

void mqttShutdown() {
  if (mqtt.connected()) {
    Serial.println("Shutting down MQTT connection");
    mqtt.loop();
    delay(10);

    net.flush();
    mqtt.disconnect();
  }
}

void notifyMqtt(String sensorName, int lastValue, int newValue, int count) {
    char payload[255];
    sprintf(
      payload,
      "{ \"sensor\": \"%s\", \"status\": %d, \"prevStatus\": %d, \"resets\": %d }",
      sensorName.c_str(),
      newValue,
      lastValue,
      reset_counter
    );
    char topic[100];
    sprintf(topic, "sensor/%s/%s", DEPLOYMENT_NAME.c_str(), sensorName.c_str());

    mqttPublish(topic, payload);
}

void sendSystemHealth() {
  Serial.println("Sending system health ...");
  char topic[100];
  sprintf(topic, "sensor/%s/%s", DEPLOYMENT_NAME.c_str(), SENSOR_NAME.c_str());

  char payload[255];
  sprintf(
    payload,
    "{ \"sensor\": \"%s\", \"resets\": %d, \"voltage\": %d }",
    SENSOR_NAME.c_str(),
    reset_counter,
    ESP.getVcc()
  );

  mqttPublish(topic, payload);
}

void setup() {
  delay(1000);
  if (!Serial) {
    Serial.begin(SERIAL_SPEED);
    while (!Serial) {}
  }

  Serial.println("Booting device...");

  state.setStateID(0);
  state.registerVar(&reset_counter);

  Serial.print("Setting up inputs... ");
  for (int i = 0; i < NO_SENSORS; i++) {
    Serial.printf("%d, ", i+ 1);
    sensors[i].registerInState(&state);
    sensors[i].setup();
    sensors[i].registerChangeCallback(notifyMqtt);
  }
  Serial.println();

  if (state.loadFromRTC()) {
    // warm boot
    Serial.println("Waking up from deep sleep...");
    // state.debugOutputRTCVars();

  } else {
    Serial.println("Running a cold boot.");

    reset_counter = 0;

    // Upon cold boot, invoke wifiManager, so it can do its work: in case there is no registered Wifi connection
    // open a new Wifi and allow selection over http interface; otherwise, connect to the known network.
    wifiConnect();
  }
}

void loop() {
  for (int i = 0; i < NO_SENSORS; i++) {
    sensors[i].readValue();

    sensors[i].toString();
    printf("Sensor value changed? %s\n", sensors[i].valueChanged() ? "yes" : "no");
  }

  if (reset_counter % 30 == 0) {
    sendSystemHealth();
  }

  printf("This has been the %dth run.\n", reset_counter++);

  Serial.println("Saving new state to RTC memory...");
  state.saveToRTC();

  // Deep sleep for 10 seconds
  mqttShutdown();
  WiFi.mode(WIFI_OFF);
  Serial.println("Going to deep sleep!\n\n\n");

  // Sleep for 10 seconds:
  system_deep_sleep_instant(10 * 1000 * 1000);
}