#include "HABPi.h"

// AHRS Constructor
AHRS::AHRS() {}

// AHRS Destructor
AHRS::~AHRS() {}

// Begin AHRS
bool AHRS::begin(i2c_bus &common_bus) {
  bool status = true;

	// Use common I2C bus
  i2c = common_bus;

  // Initialize the accel and mag sensor
  if(!accelmag.begin(ACCEL_RANGE_2G, i2c)) {
    // There was a problem detecting the FXOS8700 ... check your connections
    Module::logger.error("FXOS8700 not detected");
    status = false;
  }
  
  // Initialize the gyro sensor
  if(!gyro.begin(GYRO_RANGE_2000DPS, i2c)) {
    // There was a problem detecting the FXAS21002C ... check your connections
    Module::logger.error("FXAS21002C not detected");
    status = false;
  }

  // Abort if we cannot connect to either sensor component
  if(status == false) {
  	return status;
  }

  if (Global::Debug) {
  	sensor_t asensor, msensor, gsensor;

    // Print Accelerometer sensor details.
    accelmag.getSensor(&asensor, &msensor);
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Accelerometer" << std::endl;
    std::cout << "Sensor:     " << asensor.name << std::endl;
    std::cout << "Driver Ver: " << asensor.version << std::endl;
    std::cout << "Unique ID:  " << asensor.sensor_id << std::endl;
    std::cout << "Max Value:  " << asensor.max_value << std::endl;
    std::cout << "Min Value:  " << asensor.min_value << std::endl;
    std::cout << "Resolution: " << asensor.resolution << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << std::endl;

    // Print Magnetometer sensor details.
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Magnetometer" << std::endl;
    std::cout << "Sensor:     " << msensor.name << std::endl;
    std::cout << "Driver Ver: " << msensor.version << std::endl;
    std::cout << "Unique ID:  " << msensor.sensor_id << std::endl;
    std::cout << "Max Value:  " << msensor.max_value << std::endl;
    std::cout << "Min Value:  " << msensor.min_value << std::endl;
    std::cout << "Resolution: " << msensor.resolution << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << std::endl;

    // Print Gyroscope sensor details.
    gyro.getSensor(&gsensor);
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Gyroscope" << std::endl;
    std::cout << "Sensor:     " << gsensor.name << std::endl;
    std::cout << "Driver Ver: " << gsensor.version << std::endl;
    std::cout << "Unique ID:  " << gsensor.sensor_id << std::endl;
    std::cout << "Max Value:  " << gsensor.max_value << std::endl;
    std::cout << "Min Value:  " << gsensor.min_value << std::endl;
    std::cout << "Resolution: " << gsensor.resolution << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << std::endl;
  }

  // Filter expects 70 samples per second
  // Based on a Bluefruit M0 Feather ... rate should be adjuted for other MCUs
  filter.begin(70);

	return status;
}

// Update AHRS
void AHRS::update(float &roll, float &pitch, float &heading) {
  // Get new sensor events
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
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();
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
}