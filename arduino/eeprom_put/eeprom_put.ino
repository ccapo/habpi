/***
    eeprom_put example.

    This shows how to use the EEPROM.put() method.
    Also, this sketch will pre-set the EEPROM data for the
    example sketch eeprom_get.

    Note, unlike the single byte version EEPROM.write(),
    the put method will use update semantics. As in a byte
    will only be written to the EEPROM if the data is actually
    different.

    Written by Christopher Andrews 2015
    Released under MIT licence.
***/

#include <EEPROM.h>

struct GPSData {
  byte msg_type, msg_index, msg_total;
  float lat, lon, alt;
};

void setup() {

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  int eeAddress = 0;   //Location we want the data to be put.

  /** Put is designed for use with custom structures also. **/

  //Data to store.
  GPSData gpsData = {
    1, 0, 1,
    -80.4941693f, 43.4578917f, 336.0f
  };

  Serial.print("Size of GPSData struct: ");
  Serial.println(sizeof(GPSData));

  EEPROM.put(eeAddress, gpsData);
  Serial.print("Written GPS data type!");
}

void loop() {
  /* Empty loop */
}
