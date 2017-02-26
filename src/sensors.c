#include "habpi.h"

/* The camera image number */
int image_number = 0;

/* Sensors Initialization */
void sensors_init() {
  /* Initialize Battery Sensor */
  battery_init();

  /* Initialize GPS */
  gps_init();

  /* Initialize Temperature and Pressure Sensor */
  temperature_pressure_init();
  
  /* Initialize Magnetometer Sensor */
  magnetometer_init();
  
  /* Initialize Camera */
  camera_init();

  /* Initialize Radio */
  radio_init();
}

/* Sensors Update */
void sensors_update(sqlite3 *db) {
  sensor_data_t sensor_data;
  
  /* Update Battery Sensor */
  battery_update(db, &sensor_data);
  
  /* Update GPS */
  gps_update(db, &sensor_data);

  /* Update Temperature and Pressure Sensor */
  temperature_pressure_update(db, &sensor_data);
  
  /* Update Magnetometer Sensor */
  magnetometer_update(db, &sensor_data);
  
  /* Update Camera */
  camera_update(&sensor_data);
  
  /* Update Radio */
  radio_update(sensor_data);
}

/* Battery Initialzation */
void battery_init() {
  logger(INFO, "Battery Sensor Initialized");
}

/* Battery Update */
void battery_update(sqlite3 *db, sensor_data_t *sensor_data) {
  /* Store Battery Data */
  sensor_data->bat_volt = 3.9;
  logger(INFO, "Battery Sensor Update");
}

/* GPS Initialzation */
void gps_init() {
  logger(INFO, "GPS Initialized");
}

/* GPS Update */
void gps_update(sqlite3 *db, sensor_data_t *sensor_data) {
  /* Store GPS Data */
  sensor_data->gps_lat = 45.0;
  sensor_data->gps_lon = -80.0;
  sensor_data->gps_alt = 1500.0;
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
void temperature_pressure_update(sqlite3 *db, sensor_data_t *sensor_data) {
  float t, p, a;
  char data[DATA_STR_MAX];
  message_type_id_t message_type_id = MSG_NONE;
  
  /* Read temperature, pressure and then compute altitude */
  t = analogRead(BMP180_PINBASE)/10.0;
  p = analogRead(BMP180_PINBASE + 1)/10.0;
  a = ALPHAINV*(1.0 - pow(p/P0, BETAINV))/100.0;

  /* Debuggin Info */
  if(DEBUG_MODE) {
    printf("Temp = %f C", t);
    printf("Pres = %f hPa", p);
    printf("Alt = %f m", a);
  }

  /* Store Temperature, Pressure and Altitude Data */
  sensor_data->temp = t;
  sensor_data->baro = p;
  sensor_data->baro_alt = a;

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
  sprintf(data, "%f", a);
  insertRecord(db, message_type_id, data);
}

/* Magnetometer Sensor Initialzation */
void magnetometer_init() {
  logger(INFO, "Magnetometer Sensor Initialized");
}

/* Magnetometer Sensor Update */
void magnetometer_update(sqlite3 *db, sensor_data_t *sensor_data) {
  /* Store Magnetometer/Compass Data */
  sensor_data->magx = 4.50;
  sensor_data->magy = 80.0;
  sensor_data->magz = 15.0;
  sensor_data->mag_pitch = 45.0;
  sensor_data->mag_roll = -80.0;
  sensor_data->mag_heading = 150.0;
  logger(INFO, "Magnetometer Sensor Update");
}

/* Camera Initialzation */
void camera_init() {
  logger(INFO, "Camera Initialized");
}

/* Camera Update */
void camera_update(sensor_data_t *sensor_data) {
  /* Store image filename */
  sprintf(sensor_data->image_filename, "images/image_%06d.png", ++image_number);
  logger(INFO, "Camera Update");
}

/* Radio Initialzation */
void radio_init() {
  logger(INFO, "Radio Initialized");
}

/* Radio Update */
void radio_update(sensor_data_t sensor_data) {
  char msg[STR_MAX];

  /* Broadcast GPS, sensor data and image */
  if(DEBUG_MODE) {
    sprintf(msg, "[BAT]: %f", sensor_data.bat_volt);
    logger(DEBUG, msg);
    sprintf(msg, "[GPS]: %f,%f,%f", sensor_data.gps_lat, sensor_data.gps_lon, sensor_data.gps_alt);
    logger(DEBUG, msg);
    sprintf(msg, "[TPA]: %f,%f,%f", sensor_data.temp, sensor_data.baro, sensor_data.baro_alt);
    logger(DEBUG, msg);
    sprintf(msg, "[MAG]: %f,%f,%f,%f,%f,%f", sensor_data.magx, sensor_data.magy, sensor_data.magz, sensor_data.mag_pitch, sensor_data.mag_roll, sensor_data.mag_heading);
    logger(DEBUG, msg);
    sprintf(msg, "[CAM]: %s", sensor_data.image_filename);
    logger(DEBUG, msg);
  }
  logger(INFO, "Radio Update");
}
