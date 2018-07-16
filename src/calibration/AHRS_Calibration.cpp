#include "Adafruit_Sensor.h"
#include "i2c_bus.h"
#include "FXOS8700.h"
#include "FXAS21002C.h"

const std::string I2CPath = "/dev/i2c-1";

int main() {
  // Assign a unique ID to this sensor at the same time
  FXOS8700 accelmag = FXOS8700(0x8700A, 0x8700B);
  FXAS21002C gyro = FXAS21002C(0x0021002C);
  sensor_t asensor, msensor, gsensor;
  i2c_bus i2c;
  uint32_t delayMS = 50;
  
  std::cout << "AHRS Calibration" << std::endl;
  
  // Connect to I2C
  i2c.open(I2CPath);

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
  
  for(;;) {
    // Get a new sensor events
    sensors_event_t aevent, mevent, gevent;
    bool amstatus = accelmag.getEvent(&aevent, &mevent);
    bool gstatus = gyro.getEvent(&gevent);
    
    if (amstatus == true && gstatus == true) {
      // Display the accel results (acceleration is measured in m/s^2)
      std::cout << "Raw:" << aevent.acceleration.x << "," << aevent.acceleration.y << "," << aevent.acceleration.z;
      
      // Display the gyro results (speed is measured in rad/s)
      std::cout << "," << gevent.gyro.x << "," << gevent.gyro.y << "," << gevent.gyro.z;

      // Display the mag results (mag data is in uTesla)
      std::cout << "," << mevent.magnetic.x << "," << mevent.magnetic.y << "," << mevent.magnetic.z << std::endl;
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
