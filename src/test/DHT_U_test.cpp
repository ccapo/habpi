// DHT Temperature & Humidity Sensor
// Unified Sensor Library Example
// Written by Tony DiCola for Adafruit Industries
// Released under an MIT license.

// Depends on the following Arduino libraries:
// - Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

#include "Adafruit_Sensor.h"
#include "DHT.h"
#include "DHT_U.h"
#include <iostream>

#define DHTPIN 7     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302, AM2321)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.

int main() {
  wiringPiSetup();
  
  DHT_Unified dht(DHTPIN, DHTTYPE, 0, 1, 2);
  sensor_t sensor;
  uint32_t delayMS;
  
  // Initialize DHT sensor
  dht.begin();
  
  std::cout << "DHT Unified Sensor Test" << std::endl;
  
  // Print temperature sensor details.
  dht.temperature().getSensor(&sensor);

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

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  std::cout << "------------------------------------" << std::endl;
  std::cout << "Humidity" << std::endl;
  std::cout << "Sensor:     " << sensor.name << std::endl;
  std::cout << "Driver Ver: " << sensor.version << std::endl;
  std::cout << "Unique ID:  " << sensor.sensor_id << std::endl;
  std::cout << "Max Value:  " << sensor.max_value << "%" << std::endl;
  std::cout << "Min Value:  " << sensor.min_value << "%" << std::endl;
  std::cout << "Resolution: " << sensor.resolution << "%" << std::endl; 
  std::cout << "------------------------------------" << std::endl;
  std::cout << std::endl;

  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  
  for (;;) {
    // Delay between measurements.
    delay(delayMS);
    
    // Get temperature event and print its value.
    sensors_event_t event;  
    float t = -999.0, h = -999.0;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      std::cerr << "Error: Unable to read temperature!" << std::endl;
    } else {
      t = event.temperature;
    }

    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      std::cerr << "Error: Unable to read humidity!" << std::endl;
    } else {
      h = event.relative_humidity;
    }
    
    std::cout << "Temperature: " << event.temperature << " C, Rel. Humidity: " << event.relative_humidity << "%" << std::endl;
  }
  
  return 0;
}
