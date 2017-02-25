#include "habpi.h"

/* Sensors Initialization */
void sensors_init() {}

/* Sensors Update */
void sensors_update() {}

/* GPS Initialzation */
void gps_init() {
  logger(INFO, "GPS Initialized");
}

/* GPS Update */
void gps_update(sqlite3 *db) {
  logger(INFO, "GPS Update");
}

/* Temperature and Pressure Sensor Initialization */
void temperature_pressure_init() {
  int rc;

  /* Setup BMP180 Sensor */
  rc = bmp180Setup(BMP180_PINBASE);
  if(rc != TRUE) logger(ERROR, "Temperature and Pressure Sensor Initialization Failed: BMP180");
  logger(INFO, "Temperature and Pressure Sensor Initialized: BMP180");
}

/* Temperature and Pressure Sensor Update */
void temperature_pressure_update(sqlite3 *db) {
  float t, p, h;
  char data[DATA_STR_MAX];
  message_type_id_t message_type_id = MSG_NONE;
  
  /* Read temperature, pressure and then compute altitude */
  t = analogRead(BMP180_PINBASE)/10.0;
  p = analogRead(BMP180_PINBASE + 1)/10.0;
  h = ALPHAINV*(1.0 - pow(p/P0, BETAINV))/100.0;

  /* Debuggin Info */
  if(DEBUG_MODE) {
    printf("Temp = %f C", t);
    printf("Pres = %f hPa", p);
    printf("Alt = %f m", h);
  }

  /* Set message type, message data and insert record into DB */
  message_type_id = MSG_TEMP;
  sprintf(data, "%f", t);
  insertRecord(db, message_type_id, data);

  /* Set message type, message data and insert record into DB */
  message_type_id = MSG_BARO;
  sprintf(data, "%f", p);
  insertRecord(db, message_type_id, data);

  /* Set message type, message data and insert record into DB */
  message_type_id = MSG_BARO_ALT;
  sprintf(data, "%f", h);
  insertRecord(db, message_type_id, data);
}

/* Magnetometer Sensor Initialzation */
void magnetometer_init() {
  logger(INFO, "Magnetometer Sensor Initialized");
}

/* Magnetometer Sensor Update */
void magnetometer_update(sqlite3 *db) {
  logger(INFO, "Magnetometer Sensor Update");
}

/* Radio Initialzation */
void radio_init() {
  logger(INFO, "Radio Initialized");
}

/* Radio Update */
void radio_update(sqlite3 *db) {
  logger(INFO, "Radio Update");
}

/* Camera Initialzation */
void camera_init() {
  logger(INFO, "Camera Initialized");
}

/* Camera Update */
void camera_update(sqlite3 *db) {
  logger(INFO, "Camera Update");
}
