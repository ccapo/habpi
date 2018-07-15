#pragma once

#include <iostream>
#include <stdint.h>
#include "Adafruit_Sensor.h"
#include "i2c_bus.h"
#include "FXOS8700.h"
#include "FXAS21002C.h"
#include "Mahony.h"
#include "Madgwick.h"

class AHRS {
public:
	// AHRS Constructor
	AHRS();

	// AHRS Destructor
	~AHRS();

	// Begin AHRS sensors
	bool begin(i2c_bus &common_bus);

	// Update AHRS sensors
	void update(float &roll, float &pitch, float &heading);

	// Static Constants

  // Mag calibration values are calculated via ahrs_calibration.
  // These values must be determined for each board/environment.
  // See the image in this sketch folder for the values used
  // below.

	// Offsets applied to raw x/y/z mag values
  static const constexpr float mag_offsets[3] = { 0.93F, -7.47F, -35.23F };

  // Soft iron error compensation matrix
  static const constexpr float mag_softiron_matrix[3][3] = {{ 0.943,  0.011,  0.020 },
                                    												{ 0.022,  0.918, -0.008 },
                                    												{ 0.020, -0.008,  1.156 }};

  static const constexpr float mag_field_strength = 50.23F;

  // Offsets applied to compensate for gyro zero-drift error for x/y/z
  // Raw values converted to rad/s based on 250dps sensitiviy (1 lsb = 0.00875 rad/s)
  static const constexpr float rawToDPS = 0.00875F;
  static const constexpr float dpsToRad = 0.017453293F;
  static const constexpr float gyro_zero_offsets[3] = { 175.0F * rawToDPS * dpsToRad,
                                												-729.0F * rawToDPS * dpsToRad,
                                 												101.0F * rawToDPS * dpsToRad };

private:
  FXOS8700 accelmag;
  FXAS21002C gyro;
  i2c_bus i2c;

  // Mahony is lighter weight as a filter and should be used
  // on slower systems
  Mahony filter;
  //Madgwick filter;
};