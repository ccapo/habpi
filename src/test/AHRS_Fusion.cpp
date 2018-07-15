#include "Adafruit_Sensor.h"
#include "i2c_bus.h"
#include "FXOS8700.h"
#include "FXAS21002C.h"
#include "Mahony.h"
#include "Madgwick.h"

int main() {
  // Assign a unique ID to this sensor at the same time
  FXOS8700 accelmag = FXOS8700(0x8700A, 0x8700B);
  FXAS21002C gyro = FXAS21002C(0x0021002C);
  sensor_t asensor, msensor, gsensor;
  i2c_bus i2c;
  uint32_t delayMS = 50;
  
  // Mahony is lighter weight as a filter and should be used
  // on slower systems
  Mahony filter;
  //Madgwick filter;
  
  std::cout << "AHRS Fusion" << std::endl;
  
  // Connect to I2C
  i2c.open(I2C_DEVICE);

  // Initialize the accel and mag sensor
  if(!accelmag.begin(ACCEL_RANGE_2G, i2c)) {
    // There was a problem detecting the FXOS8700 ... check your connections
    std::cout << "Ooops, no FXOS8700 detected ... Check your connections!" << std::endl;
    return 1;
  }
  
  // Initialize the gyro sensor
  if(!gyro.begin(GYRO_RANGE_2000DPS, i2c)) {
    // There was a problem detecting the FXAS21002C ... check your connections
    std::cout << "Ooops, no FXAS21002C detected ... Check your connections!" << std::endl;
    return 1;
  }
  
  // Filter expects 70 samples per second
  // Based on a Bluefruit M0 Feather ... rate should be adjuted for other MCUs
  filter.begin(70);
  
  // Mag calibration values are calculated via ahrs_calibration.
  // These values must be determined for each board/environment.
  // See the image in this sketch folder for the values used
  // below.

  // Offsets applied to raw x/y/z mag values
  float mag_offsets[3] = { 0.93F, -7.47F, -35.23F };

  // Soft iron error compensation matrix
  float mag_softiron_matrix[3][3] = { {  0.943,  0.011,  0.020 },
                                    {  0.022,  0.918, -0.008 },
                                    {  0.020, -0.008,  1.156 } };

  float mag_field_strength = 50.23F;

  // Offsets applied to compensate for gyro zero-drift error for x/y/z
  // Raw values converted to rad/s based on 250dps sensitiviy (1 lsb = 0.00875 rad/s)
  float rawToDPS = 0.00875F;
  float dpsToRad = 0.017453293F;
  float gyro_zero_offsets[3] = { 175.0F * rawToDPS * dpsToRad,
                                -729.0F * rawToDPS * dpsToRad,
                                 101.0F * rawToDPS * dpsToRad };
  
  for(;;) {
    // Get a new sensor events
    sensors_event_t aevent, mevent, gevent;
    bool amstatus = accelmag.getEvent(&aevent, &mevent);
    bool gstatus = gyro.getEvent(&gevent);
    
    if (amstatus == true && gstatus == true) {
      // Apply mag offset compensation (base values in uTesla)
      float x = mevent.magnetic.x - mag_offsets[0];
      float y = mevent.magnetic.y - mag_offsets[1];
      float z = mevent.magnetic.z - mag_offsets[2];

      // Apply mag soft iron error compensation
      float mx = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];
      float my = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
      float mz = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];

      // Apply gyro zero-rate error compensation
      float gx = gevent.gyro.x + gyro_zero_offsets[0];
      float gy = gevent.gyro.y + gyro_zero_offsets[1];
      float gz = gevent.gyro.z + gyro_zero_offsets[2];

      // The filter library expects gyro data in degrees/s, but adafruit sensor
      // uses rad/s so we need to convert them first (or adapt the filter lib
      // where they are being converted)
      gx *= 180.0F / M_PI;
      gy *= 180.0F / M_PI;
      gz *= 180.0F / M_PI;

      // Update the filter
      filter.update(gx, gy, gz, aevent.acceleration.x, aevent.acceleration.y, aevent.acceleration.z, mx, my, mz);

      // Print the orientation filter output
      // Note: To avoid gimbal lock you should read quaternions not Euler
      // angles, but Euler angles are used here since they are easier to
      // understand looking at the raw values. See the ble fusion sketch for
      // and example of working with quaternion data.
      float roll = filter.getRoll();
      float pitch = filter.getPitch();
      float heading = filter.getYaw();
      std::cout << "Heading: " << heading << ", Pitch: " << pitch << ", Roll: " << roll << std::endl;

      // Print the orientation filter output in quaternions.
      // This avoids the gimbal lock problem with Euler angles when you get
      // close to 180 degrees (causing the model to rotate or flip, etc.)
      /*float qw, qx, qy, qz;
      filter.getQuaternion(&qw, &qx, &qy, &qz);
      std::cout << "qw: " << qw << ", qx: " << qx << ", qy: " << qy << ", qz: " << qz << std::endl;*/
    } else {
      std::cerr << "Error: Unable to get valid FXOS8700 or FXAS21002C sensor data" << std::endl;
    }

    // Delay between measurements.
    delay(delayMS);
  }
  
  // Disconnect from I2C
  i2c.close();
  
  return 0;
}
