#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    String name;
    int readPin;
    int powerPin;
    int counter;
    int lastValue;

  private:
    void (* callbackFunc)(String sensorname, int lastVal, int newVal, int count);

  public:
    Sensor(String name, int readPin, int powerPin);
    void setup();
    void readValue();
    void registerChangeCallback(void (* callbackFunc)(String sensorname, int lastVal, int newVal, int count));
};

#endif
