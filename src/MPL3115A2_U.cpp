/**
 * MPL3115A2 Barometric Pressure Sensor Library
 * By: Nathan Seidle
 * SparkFun Electronics
 * Date: September 22nd, 2013
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 *
 * This library allows an Arduino to read from the MPL3115A2 low-cost high-precision pressure sensor.
 *
 * If you have feature suggestions or need support please use the github support page: https://github.com/sparkfun/MPL3115A2_Breakout
 *
 * Hardware Setup: The MPL3115A2 lives on the I2C bus. Attach the SDA pin to A4, SCL to A5. Use inline 10k resistors
 * if you have a 5V board. If you are using the SparkFun breakout board you *do not* need 4.7k pull-up resistors 
 * on the bus (they are built-in).
 */
#include "HABPi.h"

// Default Constructor
MPL3115A2_Unified::MPL3115A2_Unified():
  _mpl(),
  _temp(this, 1),
  _pressure(this, 2),
  _altitude(this, 3)
{}

// Constructor
MPL3115A2_Unified::MPL3115A2_Unified(int32_t tempSensorId=1, int32_t baroSensorId=2, int32_t altSensorId=3):
  _mpl(),
  _temp(this, tempSensorId),
  _pressure(this, baroSensorId),
  _altitude(this, altSensorId)
{}

// Destructor
MPL3115A2_Unified::~MPL3115A2_Unified() {}

/**
 * Starts I2C communication and runs pressure calibration
 */
bool MPL3115A2_Unified::begin(i2c_bus &common_bus)
{
  // Start I2C communication
  _mpl.begin(common_bus);
  
  // Set Oversample to the recommended 128
  _mpl.setOversampleRate(7);
  
  // Enable all three pressure and temp event flags 
  _mpl.enableEventFlags();
  
  // Set to zero all the sensor offsets and input values:
	_mpl.setOffsetTemperature(0);
	_mpl.setOffsetPressure(0);
	_mpl.setOffsetAltitude(0);
	_mpl.setBarometricInput(0.0);
	
  // Now perform the calibration

	// Calculate pressure for current ALTITUDE_REFERENCE altitude by averaging a few readings
  Module::logger.info("Starting Pressure Calibration");
	
	// This function calculates a new sea level pressure ref value
	// it will NOT change the sensor registers
	// see below setBarometricInput() where that value is actually set
	// in the registers. the sensor will start using it just after.
	_mpl.runCalibration(ALTITUDE_REFERENCE);

	//std::cout << "Calculated Sea Level Pressure: " << mpl.calculated_sea_level_press << " Pa" << std::endl;
	//std::cout << "Calculated Elevation Offset: " << mpl.elevation_offset << " m" << std::endl;

	// This configuration option calibrates the sensor according to
	// the sea level pressure for the measurement location (2 Pa per LSB)
	// The default value for "Barometric input for Altitude calculation" is 101,326 Pa
	
	_mpl.setBarometricInput(_mpl.calculated_sea_level_press);

	// Optional temperature offset:
	//_mpl.setOffsetTemperature( (int8_t) (0.65 / 0.0625) );

  // Pause for 2 seconds
	delay(2000);

	// Calibration completed
  Module::logger.info("Completed Pressure Calibration");

  // Debugging Information
  if (Global::Debug) {
    sensor_t sensor;

    // Print temperature sensor details.
    _temp.getSensor(&sensor);
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Temperature" << std::endl;
    std::cout << "Sensor:     " << sensor.name << std::endl;
    std::cout << "Driver Ver: " << sensor.version << std::endl;
    std::cout << "Unique ID:  " << sensor.sensor_id << std::endl;
    std::cout << "Max Value:  " << sensor.max_value << " C" << std::endl;
    std::cout << "Min Value:  " << sensor.min_value << " C" << std::endl;
    std::cout << "Resolution: " << sensor.resolution << " C" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << std::endl;

    // Print pressure sensor details.
    _pressure.getSensor(&sensor);
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Pressure" << std::endl;
    std::cout << "Sensor:     " << sensor.name << std::endl;
    std::cout << "Driver Ver: " << sensor.version << std::endl;
    std::cout << "Unique ID:  " << sensor.sensor_id << std::endl;
    std::cout << "Max Value:  " << sensor.max_value << " kPa" << std::endl;
    std::cout << "Min Value:  " << sensor.min_value << " kPa" << std::endl;
    std::cout << "Resolution: " << sensor.resolution << " kPa" << std::endl; 
    std::cout << "------------------------------------" << std::endl;
    std::cout << std::endl;

    // Print altitude sensor details.
    _altitude.getSensor(&sensor);
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Altitude" << std::endl;
    std::cout << "Sensor:     " << sensor.name << std::endl;
    std::cout << "Driver Ver: " << sensor.version << std::endl;
    std::cout << "Unique ID:  " << sensor.sensor_id << std::endl;
    std::cout << "Max Value:  " << sensor.max_value << " km" << std::endl;
    std::cout << "Min Value:  " << sensor.min_value << " km" << std::endl;
    std::cout << "Resolution: " << sensor.resolution << " km" << std::endl; 
    std::cout << "------------------------------------" << std::endl;
    std::cout << std::endl;
  }

  return true;
}

/**
 * Get update of sensor values
 */
void MPL3115A2_Unified::update(float &temperature, float &pressure, float &altitude) {
  // Get new sensor events
  sensors_event_t tevent, pevent, aevent;
  bool tstatus = _temp.getEvent(&tevent);
  bool pstatus = _pressure.getEvent(&pevent);
  bool astatus = _altitude.getEvent(&aevent);

  if (tstatus == true && pstatus == true && astatus == true) {
    temperature = tevent.temperature;
    pressure = pevent.pressure;
    altitude = aevent.distance;
    std::cout << "Temperature: " << temperature << " C, Pressure: " << pressure << " kPa, Altitude: " << altitude << " m" << std::endl;
  } else {
    Module::logger.error("Error: Unable to get valid MPL3115A2 sensor data");
  }
}

/**
 * Sets sensor name
 */
void MPL3115A2_Unified::setName(sensor_t *sensor) {
  strncpy(sensor->name, "MPL3115A2", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name)- 1] = 0;
}

/**
 * Sets minimum sensor delay
 */
void MPL3115A2_Unified::setMinDelay(sensor_t *sensor) {
  sensor->min_delay = 1000000L;  // 1 seconds (in microseconds)
}

/**
 * Constructor for temperature sensor
 */
MPL3115A2_Unified::Temperature::Temperature(MPL3115A2_Unified *parent, int32_t id):
  _parent(parent),
  _id(id)
{}

/**
 * Gets temperature sensor data
 */ 
bool MPL3115A2_Unified::Temperature::getEvent(sensors_event_t *event) {
  // Clear event definition.
  memset(event, 0, sizeof(sensors_event_t));
  // Populate sensor reading values.
  event->version     = sizeof(sensors_event_t);
  event->sensor_id   = _id;
  event->type        = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  event->timestamp   = millis();
  event->temperature = _parent->_mpl.readTemp();
  
  return true;
}

/**
 * Gets temperature sensor information
 */
void MPL3115A2_Unified::Temperature::getSensor(sensor_t *sensor) {
  // Clear sensor definition.
  memset(sensor, 0, sizeof(sensor_t));
  // Set sensor name.
  _parent->setName(sensor);
  // Set version and ID
  sensor->version     = MPL3115A2_SENSOR_VERSION;
  sensor->sensor_id   = _id;
  // Set type and characteristics.
  sensor->type        = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  _parent->setMinDelay(sensor);
  // Set range and resolution
  sensor->max_value   = 85.0F;
  sensor->min_value   = -40.0F;
  sensor->resolution  = 3.0F;
}

/**
 * Constructor for pressure sensor
 */
MPL3115A2_Unified::Pressure::Pressure(MPL3115A2_Unified *parent, int32_t id):
  _parent(parent),
  _id(id)
{}

/**
 * Gets pressure sensor data
 */
bool MPL3115A2_Unified::Pressure::getEvent(sensors_event_t *event) {
  // Clear event definition.
  memset(event, 0, sizeof(sensors_event_t));
  // Populate sensor reading values.
  event->version           = sizeof(sensors_event_t);
  event->sensor_id         = _id;
  event->type              = SENSOR_TYPE_PRESSURE;
  event->timestamp         = millis();
  event->pressure          = _parent->_mpl.readPressure()/1000.0;
  
  return true;
}

/**
 * Gets pressure sensor information
 */
void MPL3115A2_Unified::Pressure::getSensor(sensor_t *sensor) {
  // Clear sensor definition.
  memset(sensor, 0, sizeof(sensor_t));
  // Set sensor name.
  _parent->setName(sensor);
  // Set version and ID
  sensor->version     = MPL3115A2_SENSOR_VERSION;
  sensor->sensor_id   = _id;
  // Set type and characteristics.
  sensor->type        = SENSOR_TYPE_PRESSURE;
  _parent->setMinDelay(sensor);
  // Set range and resolution
  sensor->max_value   = 110.0F;
  sensor->min_value   = 20.0F;
  sensor->resolution  = 0.0015F;
}

/**
 * Constructor for altitude sensor
 */
MPL3115A2_Unified::Altitude::Altitude(MPL3115A2_Unified *parent, int32_t id):
  _parent(parent),
  _id(id)
{}

/**
 * Gets altitude sensor data
 */
bool MPL3115A2_Unified::Altitude::getEvent(sensors_event_t *event) {
  // Clear event definition.
  memset(event, 0, sizeof(sensors_event_t));
  // Populate sensor reading values.
  event->version           = sizeof(sensors_event_t);
  event->sensor_id         = _id;
  event->type              = SENSOR_TYPE_PRESSURE;
  event->timestamp         = millis();
  event->distance          = _parent->_mpl.readAltitude();
  
  return true;
}

/**
 * Gets altitude sensor information
 */
void MPL3115A2_Unified::Altitude::getSensor(sensor_t *sensor) {
  // Clear sensor definition.
  memset(sensor, 0, sizeof(sensor_t));
  // Set sensor name.
  _parent->setName(sensor);
  // Set version and ID
  sensor->version     = MPL3115A2_SENSOR_VERSION;
  sensor->sensor_id   = _id;
  // Set type and characteristics.
  sensor->type        = SENSOR_TYPE_PRESSURE;
  _parent->setMinDelay(sensor);
  // Set range and resolution
  sensor->max_value   = 11.0F;
  sensor->min_value   = -0.70F;
  sensor->resolution  = 0.0003F;
}
