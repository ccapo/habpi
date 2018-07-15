/**
 * This is a library for the FXAS21002C Gyroscope
 *
 * Designed specifically to work with the Adafruit FXAS21002C Breakout
 * ----> https://www.adafruit.com/products
 *
 * These sensors use I2C to communicate, 2 pins (I2C)
 * are required to interface.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Kevin "KTOWN" Townsend for Adafruit Industries.
 * BSD license, all text above must be included in any redistribution
 */
#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <wiringPi.h>
#include "Adafruit_Sensor.h"
#include "i2c_bus.h"

// Sensor IDs
#define GYROSENSORID             (0x0021002C)

// I2C ADDRESS/BITS AND SETTINGS
#define FXAS21002C_ADDRESS       (0x21)       // 0100001
#define FXAS21002C_ID            (0xD7)       // 1101 0111
#define GYRO_SENSITIVITY_250DPS  (0.0078125F) // Table 35 of datasheet
#define GYRO_SENSITIVITY_500DPS  (0.015625F)  // ..
#define GYRO_SENSITIVITY_1000DPS (0.03125F)   // ..
#define GYRO_SENSITIVITY_2000DPS (0.0625F)    // ..

// REGISTERS
typedef enum {
                                               // DEFAULT    TYPE
  GYRO_REGISTER_STATUS              = 0x00,
  GYRO_REGISTER_OUT_X_MSB           = 0x01,
  GYRO_REGISTER_OUT_X_LSB           = 0x02,
  GYRO_REGISTER_OUT_Y_MSB           = 0x03,
  GYRO_REGISTER_OUT_Y_LSB           = 0x04,
  GYRO_REGISTER_OUT_Z_MSB           = 0x05,
  GYRO_REGISTER_OUT_Z_LSB           = 0x06,
  GYRO_REGISTER_WHO_AM_I            = 0x0C,   // 11010111   r
  GYRO_REGISTER_CTRL_REG0           = 0x0D,   // 00000000   r/w
  GYRO_REGISTER_CTRL_REG1           = 0x13,   // 00000000   r/w
  GYRO_REGISTER_CTRL_REG2           = 0x14,   // 00000000   r/w
} gyroRegisters_t;

// OPTIONAL SPEED SETTINGS
typedef enum {
  GYRO_RANGE_250DPS  = 250,
  GYRO_RANGE_500DPS  = 500,
  GYRO_RANGE_1000DPS = 1000,
  GYRO_RANGE_2000DPS = 2000
} gyroRange_t;

// RAW GYROSCOPE DATA TYPE
typedef struct gyroRawData_s {
  int16_t x;
  int16_t y;
  int16_t z;
} gyroRawData_t;

class FXAS21002C: public Adafruit_Sensor {
public:
  FXAS21002C();
  FXAS21002C(int32_t sensorID);
  ~FXAS21002C();

  bool begin(gyroRange_t rng, i2c_bus &common_bus);
  bool getEvent(sensors_event_t *event);
  void getSensor(sensor_t *sensor);

  gyroRawData_t raw; // Raw values from last sensor read

private:
  void write8(uint8_t reg, uint8_t value);
  uint8_t read8(uint8_t reg);

  // Private Variables
  gyroRange_t _range;
  int32_t _sensorID;
  i2c_bus i2c;
};
