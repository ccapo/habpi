#ifndef SENSORS_H
#define SENSORS_H

/* Pressure-Altitude Coefficient */
#define ALPHA 2.25577E-7

/* Pressure-Altitude Exponent */
#define BETA 5.25588

/* Inverse Pressure-Altitude Coefficient */
#define ALPHAINV (1.0/ALPHA)

/* Pressure-Altitude Exponent */
#define BETAINV (1.0/BETA)

/* Pressure at Sea Level [hPa] */
#define P0 1013.25

/* BMP180 Pin Base */
#define BMP180_PINBASE 64

/* Maximum length of a filename */
#define STR_MAX 64

/* Sensor Data Type*/
typedef struct {
 float temp, baro, baro_alt;
 float magx, magy, magz;
 float mag_pitch, mag_roll, mag_heading;
 float gps_lat, gps_lon, gps_alt;
 float bat_volt;
 char img_filename[STR_MAX];
} sensor_data_t;

/* The camera image number */
extern int image_number;

/* Wrapper for Sensors Initializaton */
void sensors_init();

/* Wrapper for Sensors Update */
void sensors_update(sqlite3 *db);

/* Battery Sensor Initialization */
void battery_init();

/* Battery Sensor Update */
void battery_update(sqlite3 *db, sensor_data_t *sensor_data);

/* GPS Initialization */
void gps_init();

/* GPS Update */
void gps_update(sqlite3 *db, sensor_data_t *sensor_data);

/* Temperature and Pressure Sensor Initialization */
void temperature_pressure_init();

/* Temperature and Pressure Sensor Update */
void temperature_pressure_update(sqlite3 *db, sensor_data_t *sensor_data);

/* Magnetometer Sensor Initialization */
void magnetometer_init();

/* Magnetometer Sensor Update */
void magnetometer_update(sqlite3 *db, sensor_data_t *sensor_data);

/* Camera Initialization */
void camera_init();

/* Camera Update */
void camera_update(sensor_data_t *sensor_data);

/* Radio Initialization */
void radio_init();

/* Radio Update */
void radio_update(sensor_data_t sensor_data);

#endif
