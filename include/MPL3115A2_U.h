/**
 * MPL3115A2 Barometric Pressure Sensor Library
 * By: Nathan Seidle
 * SparkFun Electronics
 * Date: September 24th, 2013
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 *
 * Get pressure, altitude and temperature from the MPL3115A2 sensor. 
 */
#pragma once

#include "Adafruit_Sensor.h"
#include "MPL3115A2.h"

// Sensor version
#define MPL3115A2_SENSOR_VERSION (1)

// Reference Altitude @ 43.4578362, -80.4920564
#define ALTITUDE_REFERENCE       (337.0)

// MPL3115A2 Unified class definition
class MPL3115A2_Unified {
  public:
  MPL3115A2_Unified();
  MPL3115A2_Unified(int32_t tempSensorId, int32_t baroSensorId, int32_t altSensorId);
  ~MPL3115A2_Unified();

  // Public Functions
  bool begin(i2c_bus &common_bus); // Gets sensor on the I2C bus.

  void update(float &temperature, float &pressure, float &altitude);
  
  class Temperature: public Adafruit_Sensor {
    public:
    Temperature(MPL3115A2_Unified *parent, int32_t id);
    bool getEvent(sensors_event_t *event);
    void getSensor(sensor_t *sensor);

    private:
    MPL3115A2_Unified *_parent;
    int32_t _id;
  };
  
  class Pressure: public Adafruit_Sensor {
    public:
    Pressure(MPL3115A2_Unified *parent, int32_t id);
    bool getEvent(sensors_event_t *event);
    void getSensor(sensor_t *sensor);

    private:
    MPL3115A2_Unified *_parent;
    int32_t _id;
  };
  
  class Altitude: public Adafruit_Sensor {
    public:
    Altitude(MPL3115A2_Unified *parent, int32_t id);
    bool getEvent(sensors_event_t *event);
    void getSensor(sensor_t *sensor);

    private:
    MPL3115A2_Unified *_parent;
    int32_t _id;
  };
  
  Temperature temperature() {
    return _temp;
  }

  Pressure pressure() {
    return _pressure;
  }
  
  Altitude altitude() {
    return _altitude;
  }

  private:
  // Private Variables
  MPL3115A2 _mpl;
  Temperature _temp;
  Pressure _pressure;
  Altitude _altitude;

  // Private Functions
  void setName(sensor_t *sensor);
  void setMinDelay(sensor_t *sensor);
};
