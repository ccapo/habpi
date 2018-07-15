/**
 * DHT library
 *
 * MIT license
 *
 * Written by Adafruit Industries
 */
#pragma once

#include "Global.h"
 
#define clockCyclesPerMicrosecond() ( 260L ) // 260 is Clock Cycle of LinkIt ONE in MHz
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

// Uncomment to enable printing out nice debug messages.
//#define DHT_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef DHT_DEBUG
  #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
  #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
  #define DEBUG_PRINT(...) {}
  #define DEBUG_PRINTLN(...) {}
#endif

// Define types of sensors.
#define DHT11  (11)
#define DHT22  (22)
#define DHT21  (21)
#define AM2301 (21)

class DHT {
public:
  DHT(uint8_t pin, uint8_t type, uint8_t count=6);
  void begin(void);
  float readTemperature(bool S=false, bool force=false);
  float convertCtoF(float);
  float convertFtoC(float);
  float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit=true);
  float readHumidity(bool force=false);
  bool read(bool force=false);

private:
  uint8_t data[5];
  uint8_t _pin, _type;
  uint32_t _lastreadtime, _maxcycles;
  bool _lastresult;

  uint32_t expectPulse(bool level);
};
