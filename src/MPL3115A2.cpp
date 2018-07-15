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

MPL3115A2::MPL3115A2() {}

MPL3115A2::~MPL3115A2() {}

/*******************************************************************************************/
// Start I2C communication
void MPL3115A2::begin(i2c_bus &common_bus) {
	// Use common I2C bus
  i2c = common_bus;
}


// Returns the number of meters above sea level
// Returns -999 if no new data is available
float MPL3115A2::readAltitude() {
  // Set mode as altimeter
  setModeAltimeter();
  delay(520);

	// Check PDR bit, if it's not set then toggle OST
  if( (IIC_Read(STATUS) & (1 << 2)) == 0) toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading

	// Wait for PDR bit, indicates we have new pressure data
	int32_t counter = 0;
	while( (IIC_Read(STATUS) & (1 << 2)) == 0) {
		if(++counter > 600) return(-999.0); // Error out after max of 512ms for a read
		delay(1);
	}

	// Read pressure registers
	uint8_t data[3];
	if (i2c.try_write_byte_and_read(MPL3115A2_ADDRESS, OUT_P_MSB, data, sizeof(data)) == -1) {
		return -999.0;
	}

	uint8_t msb, csb, lsb;
	msb = data[0];
	csb = data[1];
	lsb = data[2];

	// The least significant bytes l_altitude and l_temp are 4-bit,
	// fractional values, so you must cast the calulation in (float),
	// shift the value over 4 spots to the right and divide by 16 (since 
	// there are 16 values in 4-bits). 
	float tempcsb = (lsb >> 4)/16.0;

	float altitude = (float)( (msb << 8) | csb) + tempcsb;

	return altitude;
}

// Reads the current pressure in Pa
// Unit must be set in barometric pressure mode
// Returns -999 if no new data is available
float MPL3115A2::readPressure() {
  // Set mode for barometer
  setModeBarometer();
  delay(520);

	// Check PDR bit, if it's not set then toggle OST
	if((IIC_Read(STATUS) & (1 << 2)) == 0) toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading

	// Wait for PDR bit, indicates we have new pressure data
	int32_t counter = 0;
	while((IIC_Read(STATUS) & (1 << 2)) == 0) {
		if(++counter > 600) return(-999000.0); // Error out after max of 512ms for a read
		delay(1);
	}

	// Read pressure registers
  uint8_t data[3];
	if (i2c.try_write_byte_and_read(MPL3115A2_ADDRESS, OUT_P_MSB, data, sizeof(data)) == -1) {
		return -999000.0;
	}

	uint8_t msb, csb, lsb;
	msb = data[0];
	csb = data[1];
	lsb = data[2];
	
	toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading

	// Pressure comes back as a left shifted 20 bit number
	long pressure_whole = (long)msb<<16 | (long)csb<<8 | (long)lsb;
	pressure_whole >>= 6; // Pressure is an 18 bit number with 2 bits of decimal. Get rid of decimal portion.

	lsb &= 0x30; //B00110000; // Bits 5/4 represent the fractional component
	lsb >>= 4; // Get it right aligned
	float pressure_decimal = (float)lsb/4.0; //T urn it into fraction

	float pressure = (float)pressure_whole + pressure_decimal;

	return pressure;
}

float MPL3115A2::readTemp() {
  // Check TDR bit, if it's not set then toggle OST
	if((IIC_Read(STATUS) & (1 << 1)) == 0) toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading

	// Wait for TDR bit, indicates we have new temp data
	int32_t counter = 0;
	while( (IIC_Read(STATUS) & (1 << 1)) == 0) {
		if(++counter > 600) return(-999.0); // Error out after max of 512ms for a read
		delay(1);
	}

	// Read temperature registers
	uint8_t data[2];
	if (i2c.try_write_byte_and_read(MPL3115A2_ADDRESS, OUT_T_MSB, data, 2) == -1) {
		return -999.0;
	}

	uint8_t msb, lsb;
	msb = data[0];
	lsb = data[1];

	toggleOneShot(); // Toggle the OST bit causing the sensor to immediately take another reading

  // Negative temperature fix by D.D.G.
	uint16_t foo = 0;
  bool negSign = false;

  // Check for 2s compliment
	if(msb > 0x7F) {
    foo = ~((msb << 8) + lsb) + 1;  // Two’s complement
    msb = foo >> 8;
    lsb = foo & 0x00F0; 
    negSign = true;
	}

	// The least significant bytes l_altitude and l_temp are 4-bit,
	// fractional values, so you must cast the calulation in (float),
	// shift the value over 4 spots to the right and divide by 16 (since 
	// there are 16 values in 4-bits). 
	float templsb = (lsb >> 4)/16.0; // temp, fraction of a degree

	float temperature = (float)(msb + templsb);

	if (negSign) temperature = 0 - temperature;
	
	return temperature;
}

// Sets the mode to Barometer
// CTRL_REG1, ALT bit
void MPL3115A2::setModeBarometer() {
  uint8_t tempSetting = IIC_Read(CTRL_REG1); // Read current settings
  tempSetting &= ~(1 << 7); // Clear ALT bit
  IIC_Write(CTRL_REG1, tempSetting);
}

// Sets the mode to Altimeter
// CTRL_REG1, ALT bit
void MPL3115A2::setModeAltimeter() {
  uint8_t tempSetting = IIC_Read(CTRL_REG1); // Read current settings
  tempSetting |= (1 << 7); // Set ALT bit
  IIC_Write(CTRL_REG1, tempSetting);
}

// Puts the sensor in standby mode
// This is needed so that we can modify the major control registers
void MPL3115A2::setModeStandby() {
  uint8_t tempSetting = IIC_Read(CTRL_REG1); // Read current settings
  tempSetting &= ~(1 << 0); // Clear SBYB bit for Standby mode
  IIC_Write(CTRL_REG1, tempSetting);
}

// Puts the sensor in active mode
// This is needed so that we can modify the major control registers
void MPL3115A2::setModeActive() {
  uint8_t tempSetting = IIC_Read(CTRL_REG1); // Read current settings
  tempSetting |= (1 << 0); // Set SBYB bit for Active mode
  IIC_Write(CTRL_REG1, tempSetting);
}

// Call with a rate from 0 to 7. See page 33 for table of ratios.
// Sets the over sample rate. Datasheet calls for 128 but you can set it 
// from 1 to 128 samples. The higher the oversample rate the greater
// the time between data samples.
void MPL3115A2::setOversampleRate(uint8_t sampleRate) {
  if(sampleRate > 7) sampleRate = 7; // OS cannot be larger than 0b.0111
  sampleRate <<= 3; // Align it for the CTRL_REG1 register
  
  uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
  tempSetting &= 0xC7; //B11000111; // Clear out old OS bits
  tempSetting |= sampleRate; // Mask in new OS bits
  IIC_Write(CTRL_REG1, tempSetting);
}

// Enables the pressure and temp measurement event flags so that we can
// test against them. This is recommended in datasheet during setup.
void MPL3115A2::enableEventFlags() {
  IIC_Write(PT_DATA_CFG, 0x07); // Enable all three pressure and temp event flags 
}

// Clears then sets the OST bit which causes the sensor to immediately take another reading
// Needed to sample faster than 1 Hz
void MPL3115A2::toggleOneShot(void) {
  uint8_t tempSetting = IIC_Read(CTRL_REG1); // Read current settings
  tempSetting &= ~(1 << 1); // Clear OST bit
  IIC_Write(CTRL_REG1, tempSetting);

  tempSetting = IIC_Read(CTRL_REG1); // Read current settings to be safe
  tempSetting |= (1 << 1); // Set OST bit
  IIC_Write(CTRL_REG1, tempSetting);
}


// These are the two I2C functions in this sketch.
uint8_t MPL3115A2::IIC_Read(uint8_t regAddr) {
  // This function reads one uint8_t over IIC
  uint8_t data = 0;
  i2c.write_byte_and_read(MPL3115A2_ADDRESS, regAddr, &data, sizeof(data));
  return data;
}

void MPL3115A2::IIC_Write(uint8_t regAddr, uint8_t value) {
  // This function writes one byte over IIC
  uint8_t data[2] = {0};
  data[0] = regAddr;
  data[1] = value;
  i2c.write(MPL3115A2_ADDRESS, data, sizeof(data));
}

// added by https://github.com/mariocannistra
// the following offset routines are from Michael Lange on mbed.org
// modified for some data types and to pre-calculate the offset in °C 
// when reading the temperature offset

//! Returns the altitude offset stored in the sensor.
int8_t MPL3115A2::offsetAltitude() {
  return (int8_t) IIC_Read(OFF_H);
}

//! Sets the altitude offset stored in the sensor. The allowed offset range is from -128 to 127 meters.
void MPL3115A2::setOffsetAltitude(int8_t offset) {
  IIC_Write(OFF_H, offset);
}

//! Returns the pressure offset stored in the sensor.
float MPL3115A2::offsetPressure() {
  return (float) IIC_Read(OFF_P);
}

//! Sets the pressure offset stored in the sensor. The allowed offset range is from -128 to 127 where each LSB represents 2 Pa.
void MPL3115A2::setOffsetPressure(int8_t offset) {
  IIC_Write(OFF_P, offset);
}

//! Returns the temperature offset stored in the sensor.
float MPL3115A2::offsetTemperature() {
  return (float) IIC_Read(OFF_T) * 0.0625;
}

//! Sets the temperature offset stored in the sensor. The allowed offset range is from -128 to 127 where each LSB represents 0.0625ºC.
void MPL3115A2::setOffsetTemperature(int8_t offset) {
  IIC_Write(OFF_T, offset);
} 

// the 2 following functions comes from http://www.henrylahr.com/?p=99
// edited to merge them with the Sparkfun library

// sets the "Barometric input for Altitude calculation" (see datasheet)
void MPL3115A2::setBarometricInput(float pressSeaLevel) {
	IIC_Write(BAR_IN_MSB, (uint32_t)(pressSeaLevel / 2) >> 8);
	IIC_Write(BAR_IN_LSB, (uint32_t)(pressSeaLevel / 2) & 0xFF);
}

void MPL3115A2::runCalibration(float currentElevation) {
	float pressureAccum = 0.0;
	
	setModeBarometer(); // Measure pressure in Pascals
	setOversampleRate(7); // Set Oversample to the recommended 128 --> 512ms
	enableEventFlags(); // Enable all three pressure and temp event flags 

	for (uint8_t i = 0; i < 8; i++) {
		delay(550); // Wait for sensor to read pressure (512ms in datasheet)
		pressureAccum = pressureAccum + readPressure();
	}
	float currpress = pressureAccum / 8.0; // Average pressure over (6/550) seconds

	//std::cout << "Current average pressure: " << currpress << " Pa" << std::endl;

	// Calculate pressure at mean sea level based on the known altitude
	float powElement = pow(1.0 - (currentElevation*0.0000225577), 5.255877);
	calculated_sea_level_press = currpress / powElement;
	//std::cout << "Calculated sea level pressure: " << calculated_sea_level_press << " Pa" << std::endl;

	elevation_offset = 101325.0 - (101325.0 * powElement);
}
