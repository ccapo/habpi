/**
* MPL3115A2 Barometric Pressure Sensor Library Example Code
* By: Nathan Seidle
* SparkFun Electronics
* Date: September 24th, 2013
* License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
*
* Uses the MPL3115A2 library to display the current altitude and temperature
*
* Hardware Connections (Breakoutboard to Arduino):
* -VCC = 3.3V
* -SDA = A4 (use inline 10k resistor if your board is 5V)
* -SCL = A5 (use inline 10k resistor if your board is 5V)
* -GND = Common ground
* -INT pins can be left unconnected for this demo
*/

#include "Adafruit_Sensor.h"
#include "MPL3115A2.h"
#include "MPL3115A2_U.h"

int main() {
  // Create an instance of MPL3115A2_Unified, passing unique IDs for each sensor component
  MPL3115A2_Unified mpl(1, 2, 3);
  sensor_t sensor;
  uint32_t delayMS;

  std::cout << "MPL3115A2 Unified Sensor Test" << std::endl;

  // Initialize MPL3115A2 sensor
  mpl.begin();

  // Print temperature sensor details.
  mpl.temperature().getSensor(&sensor);
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
  mpl.pressure().getSensor(&sensor);
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
  mpl.altitude().getSensor(&sensor);
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

  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  
  delay(2000);

  for (;;) {
    // Get temperature event and print its value.
    sensors_event_t event;  
    float t = -999.0, p = -999.0, a = -999.0;
    mpl.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      std::cerr << "Error: Unable to read temperature!" << std::endl;
    } else {
      t = event.temperature;
    }

    // Get pressure event and print its value.
    mpl.pressure().getEvent(&event);
    if (isnan(event.pressure)) {
      std::cerr << "Error: Unable to read pressure!" << std::endl;
    } else {
      p = event.pressure;
    }

    // Get altitude event and print its value.
    mpl.altitude().getEvent(&event);
    if (isnan(event.distance)) {
      std::cerr << "Error: Unable to read altitude!" << std::endl;
    } else {
      a = event.distance;
    }

    std::cout << "Temperature: " << t << " C, Pressure: " << p << " kPa, Altitude: " << a << " km" << std::endl;

    // Delay between measurements.
    delay(delayMS);
  }

  return 0;
}
