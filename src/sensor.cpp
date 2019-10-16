#include <Arduino.h>
#include <RTCVars.h>
#include "sensor.h"

Sensor::Sensor(String name, int readPin, int powerPin) {
  this->name = name;
  this->readPin = readPin;
  this->powerPin = powerPin;
  this->counter = 0;
  this->lastValue = -1;
  this->value = -1;
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

void Sensor::registerInState(RTCVars *state) {
  state->registerVar(&this->value);
}

void Sensor::readValue() {

  // Before reading new value, shift old one into the corresponding
  // field, so it can be compared with later on
  this->lastValue = this->value;

  // Turn on power
  digitalWrite(this->powerPin, HIGH);

  // Wait, then read value
  delay(5);
  this->value = digitalRead(this->readPin);

  // Turn off the power
  digitalWrite(this->powerPin, LOW);

  // Check results
  if (this->value != this->lastValue) {
    Serial.printf("Sensor %s %s.\n", this->name.c_str(), this->value == HIGH ? "closed." : "open!");

    this->counter++;

    if (this->callbackFunc != NULL) {
      this->callbackFunc(this->name, this->lastValue, this->value, this->counter);
    }
  }
}

bool Sensor::valueChanged() {
  return this->value != this->lastValue;
}

void Sensor::toString() {
  Serial.printf("Sensor '%s': { last: %d, curr: %d }\n", this->name.c_str(), this->lastValue, this->value);
}
