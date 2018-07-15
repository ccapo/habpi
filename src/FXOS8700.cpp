/**
 * This is a library for the FXOS8700 Accel/Mag
 *
 * Designed specifically to work with the Adafruit FXOS8700 Breakout
 * ----> https://www.adafruit.com/products/
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
#include "HABPi.h"

// Utility function to write a byte to a register
void FXOS8700::write8(uint8_t reg, uint8_t value) {
  uint8_t data[2] = {0};
  data[0] = reg;
  data[1] = value;
  i2c.write(FXOS8700_ADDRESS, data, sizeof(data));
}

// Utility function to read a byte
uint8_t FXOS8700::read8(uint8_t reg) {
  uint8_t value = 0;
  i2c.write_byte_and_read(FXOS8700_ADDRESS, reg, &value, sizeof(value));
  return value;
}

// DEFAULT CONSTRUCTOR
FXOS8700::FXOS8700() {
  _accelSensorID = ACCSENSORID;
  _magSensorID = MAGSENSORID;
}

// CONSTRUCTOR
FXOS8700::FXOS8700(int32_t accelSensorID, int32_t magSensorID) {
  _accelSensorID = accelSensorID;
  _magSensorID = magSensorID;
}

// DESTRUCTOR
FXOS8700::~FXOS8700() {}

// Setups the HW
bool FXOS8700::begin(fxos8700AccelRange_t rng, i2c_bus &common_bus) {
  // Use common I2C bus
  i2c = common_bus;

  // Set the range the an appropriate value
  _range = rng;

  // Clear the raw sensor data
  accel_raw.x = 0;
  accel_raw.y = 0;
  accel_raw.z = 0;
  mag_raw.x = 0;
  mag_raw.y = 0;
  mag_raw.z = 0;

  // Make sure we have the correct chip ID since this checks
  // for correct address and that the IC is properly connected
  uint8_t id = read8(FXOS8700_REGISTER_WHO_AM_I);
  //std::cout << "WHO AM I? 0x" << std::hex << id << std::endl;
  if (id != FXOS8700_ID) {
    return false;
  }

  // Set to standby mode (required to make changes to this register)
  write8(FXOS8700_REGISTER_CTRL_REG1, 0x00);
  
  uint8_t sysmod = read8(FXOS8700_REGISTER_SYSMOD);
  while(static_cast<int>(sysmod) != 0x00) {
    std::cout << "SYSMOD = " << static_cast<int>(sysmod) << std::endl;
    sysmod = read8(FXOS8700_REGISTER_SYSMOD);
  }

  // Configure the accelerometer
  switch(_range) {
    case(ACCEL_RANGE_2G):
      write8(FXOS8700_REGISTER_XYZ_DATA_CFG, 0x00);
      break;
    case(ACCEL_RANGE_4G):
      write8(FXOS8700_REGISTER_XYZ_DATA_CFG, 0x01);
      break;
    case(ACCEL_RANGE_8G):
      write8(FXOS8700_REGISTER_XYZ_DATA_CFG, 0x02);
      break;
  }
  
  // Configure the magnetometer
  // Hybrid Mode, Over Sampling Rate = 16
  write8(FXOS8700_REGISTER_MCTRL_REG1, 0x1F);
  // Jump to reg 0x33 after reading 0x06
  write8(FXOS8700_REGISTER_MCTRL_REG2, 0x20);

  // High resolution
  write8(FXOS8700_REGISTER_CTRL_REG2, 0x02);
  // Active, Normal Mode, Low Noise, 100Hz in Hybrid Mode
  write8(FXOS8700_REGISTER_CTRL_REG1, 0x15);
  
  delay(100);

  return true;
}

// Gets the most recent sensor event
bool FXOS8700::getEvent(sensors_event_t *accelEvent, sensors_event_t *magEvent) {
  bool readingValid = false;

  // Clear the event
  memset(accelEvent, 0, sizeof(sensors_event_t));
  memset(magEvent, 0, sizeof(sensors_event_t));

  // Clear the raw data placeholder
  accel_raw.x = 0;
  accel_raw.y = 0;
  accel_raw.z = 0;
  mag_raw.x = 0;
  mag_raw.y = 0;
  mag_raw.z = 0;

  // Set the static metadata
  accelEvent->version   = sizeof(sensors_event_t);
  accelEvent->sensor_id = _accelSensorID;
  accelEvent->type      = SENSOR_TYPE_ACCELEROMETER;

  magEvent->version   = sizeof(sensors_event_t);
  magEvent->sensor_id = _magSensorID;
  magEvent->type      = SENSOR_TYPE_MAGNETIC_FIELD;

  // Read 13 bytes from the sensor
  uint8_t data[13] = {0};
  i2c.write_byte_and_read(FXOS8700_ADDRESS, FXOS8700_REGISTER_STATUS, data, sizeof(data));

  // TODO: Check status first!
  uint8_t status = data[0];
  uint8_t axhi = data[1];
  uint8_t axlo = data[2];
  uint8_t ayhi = data[3];
  uint8_t aylo = data[4];
  uint8_t azhi = data[5];
  uint8_t azlo = data[6];
  uint8_t mxhi = data[7];
  uint8_t mxlo = data[8];
  uint8_t myhi = data[9];
  uint8_t mylo = data[10];
  uint8_t mzhi = data[11];
  uint8_t mzlo = data[12];

  // Set sensor status
  if (static_cast<int>(status) == 0xFF) {
    readingValid = true;
  }

  // Set the timestamps
  accelEvent->timestamp = millis();
  magEvent->timestamp = accelEvent->timestamp;

  // Shift values to create properly formed integers
  // Note, accel data is 14-bit and left-aligned, so we shift two bit right
  accelEvent->acceleration.x = (int16_t)((axhi << 8) | axlo) >> 2;
  accelEvent->acceleration.y = (int16_t)((ayhi << 8) | aylo) >> 2;
  accelEvent->acceleration.z = (int16_t)((azhi << 8) | azlo) >> 2;
  magEvent->magnetic.x = (int16_t)((mxhi << 8) | mxlo);
  magEvent->magnetic.y = (int16_t)((myhi << 8) | mylo);
  magEvent->magnetic.z = (int16_t)((mzhi << 8) | mzlo);

  // Assign raw values in case someone needs them
  accel_raw.x = accelEvent->acceleration.x;
  accel_raw.y = accelEvent->acceleration.y;
  accel_raw.z = accelEvent->acceleration.z;
  mag_raw.x = magEvent->magnetic.x;
  mag_raw.y = magEvent->magnetic.y;
  mag_raw.z = magEvent->magnetic.z;

  // Convert accel values to m/s^2
  switch(_range) {
    case(ACCEL_RANGE_2G):
      accelEvent->acceleration.x *= ACCEL_MG_LSB_2G * SENSORS_GRAVITY_STANDARD;
      accelEvent->acceleration.y *= ACCEL_MG_LSB_2G * SENSORS_GRAVITY_STANDARD;
      accelEvent->acceleration.z *= ACCEL_MG_LSB_2G * SENSORS_GRAVITY_STANDARD;
      break;
    case(ACCEL_RANGE_4G):
      accelEvent->acceleration.x *= ACCEL_MG_LSB_4G * SENSORS_GRAVITY_STANDARD;
      accelEvent->acceleration.y *= ACCEL_MG_LSB_4G * SENSORS_GRAVITY_STANDARD;
      accelEvent->acceleration.z *= ACCEL_MG_LSB_4G * SENSORS_GRAVITY_STANDARD;
      break;
    case(ACCEL_RANGE_8G):
      accelEvent->acceleration.x *= ACCEL_MG_LSB_8G * SENSORS_GRAVITY_STANDARD;
      accelEvent->acceleration.y *= ACCEL_MG_LSB_8G * SENSORS_GRAVITY_STANDARD;
      accelEvent->acceleration.z *= ACCEL_MG_LSB_8G * SENSORS_GRAVITY_STANDARD;
      break;
  }

  // Convert mag values to uTesla
  magEvent->magnetic.x *= MAG_UT_LSB;
  magEvent->magnetic.y *= MAG_UT_LSB;
  magEvent->magnetic.z *= MAG_UT_LSB;

  return readingValid;
}

// Gets the sensor_t data
void FXOS8700::getSensor(sensor_t *accelSensor, sensor_t *magSensor) {
  // Clear the sensor_t object
  memset(accelSensor, 0, sizeof(sensor_t));
  memset(magSensor, 0, sizeof(sensor_t));

  // Insert the sensor name in the fixed length char array
  strncpy(accelSensor->name, "FXOS8700", sizeof(accelSensor->name) - 1);
  accelSensor->name[sizeof(accelSensor->name) - 1] = 0;
  accelSensor->version     = 1;
  accelSensor->sensor_id   = _accelSensorID;
  accelSensor->type        = SENSOR_TYPE_ACCELEROMETER;
  accelSensor->min_delay   = 500000L; // 0.5 seconds (in microseconds) or 2 Hz
  switch(_range) {
    case(ACCEL_RANGE_2G):
      accelSensor->max_value  = 2.0F * SENSORS_GRAVITY_STANDARD;
      accelSensor->min_value  = -1.999F * SENSORS_GRAVITY_STANDARD;
      accelSensor->resolution = ACCEL_MG_LSB_2G * SENSORS_GRAVITY_STANDARD;
      break;
    case(ACCEL_RANGE_4G):
      accelSensor->max_value  = 4.0F * SENSORS_GRAVITY_STANDARD;
      accelSensor->min_value  = -3.998F * SENSORS_GRAVITY_STANDARD;
      accelSensor->resolution = ACCEL_MG_LSB_4G * SENSORS_GRAVITY_STANDARD;
      break;
    case(ACCEL_RANGE_8G):
      accelSensor->max_value  = 8.0F * SENSORS_GRAVITY_STANDARD;
      accelSensor->min_value  = -7.996F * SENSORS_GRAVITY_STANDARD;
      accelSensor->resolution = ACCEL_MG_LSB_8G * SENSORS_GRAVITY_STANDARD;
      break;
  }

  strncpy (magSensor->name, "FXOS8700", sizeof(magSensor->name) - 1);
  magSensor->name[sizeof(magSensor->name) - 1] = 0;
  magSensor->version     = 1;
  magSensor->sensor_id   = _magSensorID;
  magSensor->type        = SENSOR_TYPE_MAGNETIC_FIELD;
  magSensor->min_delay   = 500000L; // 0.5 seconds (in microseconds) or 2 Hz
  magSensor->max_value   = 1200.0F;
  magSensor->min_value   = -1200.0F;
  magSensor->resolution  = 0.1F;
}

// To keep Adafruit_Sensor happy we need a single sensor interface
// When only one sensor is requested, return accel data
bool FXOS8700::getEvent(sensors_event_t *accelEvent) {
  sensors_event_t mag;
  return getEvent(accelEvent, &mag);
}

// To keep Adafruit_Sensor happy we need a single sensor interface
// When only one sensor is requested, return accel data
void FXOS8700::getSensor(sensor_t *accelSensor) {
  sensor_t mag;
  return getSensor(accelSensor, &mag);
}
