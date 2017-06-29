/***
    eeprom_get example.

    This shows how to use the EEPROM.get() method.

    To pre-set the EEPROM data, run the example sketch eeprom_put.
    This sketch will run without it, however, the values shown
    will be shown from what ever is already on the EEPROM.

    This may cause the serial object to print out a large string
    of garbage if there is no null character inside one of the strings
    loaded.

    Written by Christopher Andrews 2015
    Released under MIT licence.
***/

#include <EEPROM.h>

struct GPSData {
  byte msg_type, msg_index, msg_total;
  float lat, lon, alt;
};

void setup() {
  int eeAddress = 0; //EEPROM address to start reading from

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  /***
    Get can be used with custom structures too.
    I have separated this into an extra function.
  ***/

  GPSData gpsData; //Variable to store custom object read from EEPROM.
  EEPROM.get(eeAddress, gpsData);

  Serial.println("Read GPS data from EEPROM: ");
  Serial.print("Message Type: ");
  Serial.println(gpsData.msg_type);
  Serial.print("Message Index: ");
  Serial.println(gpsData.msg_index);
  Serial.print("Message Total: ");
  Serial.println(gpsData.msg_total);
  Serial.print("GPS LAT: ");
  Serial.println(gpsData.lat, 6);
  Serial.print("GPS LON: ");
  Serial.println(gpsData.lon, 6);
  Serial.print("GPS ALT: ");
  Serial.println(gpsData.alt, 6);
}

void loop() {
  /* Empty loop */
}
