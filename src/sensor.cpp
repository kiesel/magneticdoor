#include <Arduino.h>
#include "sensor.h"

Sensor::Sensor(String name, int readPin, int powerPin) {
  this->name = name;
  this->readPin = readPin;
  this->powerPin = powerPin;
  this->counter = 0;
  this->lastValue = LOW;
  this->callbackFunc = NULL;
}

void Sensor::setup() {
  pinMode(this->readPin, INPUT);
  pinMode(this->powerPin, OUTPUT);
  digitalWrite(this->powerPin, LOW);
}

void Sensor::registerChangeCallback(void (* callbackFunc)(String sensorname, int lastVal, int newVal, int count)) {
  this->callbackFunc = callbackFunc;
}

void Sensor::readValue() {

  // Turn on power
  digitalWrite(this->powerPin, HIGH);

  // Wait, then read value
  delay(5);
  int value = digitalRead(this->readPin);

  // Turn off the power
  digitalWrite(this->powerPin, LOW);

  // Check results
  if (value != this->lastValue) {
    if (value == HIGH) {
      Serial.println("Magnetic sensor closed.");
    } else {
      Serial.println("Magnetic sensor open!");
    }

    this->counter++;

    if (this->callbackFunc != NULL) {
      this->callbackFunc(this->name, this->lastValue, value, this->counter);
    }

    this->lastValue = value;
  }
}

