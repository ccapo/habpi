/* Message Type ID */
typedef enum {
  MSG_NONE = 0,
  MSG_SYS = 1,
  MSG_GPS_LAT = 2,
  MSG_GPS_LON = 3,
  MSG_GPS_ALT = 4,
  MSG_TEMP = 5,
  MSG_BARO = 6,
  MSG_BARO_ALT = 7,
  MSG_MAGX = 8,
  MSG_MAGY = 9,
  MSG_MAGZ = 10,
  MSG_MAG_PITCH = 11,
  MSG_MAG_ROLL = 12,
  MSG_MAG_HEADING = 13,
  MSG_BAT = 14,
  MSG_CAM = 15,
  NMSG = 16
} message_type_id_t;

#define SQL_STR_MAX 128

/* Connect to sqlite3 DB */
void connectDB(sqlite3 **db, char *dbName);

/* Disconnect from sqlite3 DB */
void disconnectDB(sqlite3 *db, char *dbName);

/* Callback function for sqlite3_exec */
int callback(void *data, int argc, char **argv, char **azColName);

/* Insert data into sqlite3 DB */
void insertRecord(sqlite3 *db, message_type_id_t message_type_id, char *data);
