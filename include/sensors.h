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

/* Sensor Data Type*/
typedef struct {
 float temp, baro, baro_alt;
 float magx, magy, magz;
 float mag_pitch, mag_roll, mag_heading;
 float gps_lat, gps_lon, gps_alt;
 float bat_volt;
} sensor_data_t;

void sensors_init();
void sensors_update();
void gps_init();
void gps_update(sqlite3 *db);
void temperature_pressure_init();
void temperature_pressure_update(sqlite3 *db);
void magnetometer_init();
void magnetometer_update(sqlite3 *db);
void camera_init();
void camera_update(sqlite3 *db);
void radio_init();
void radio_update(sqlite3 *db);

#endif
