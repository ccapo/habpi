#pragma once

#include "Global.h"

// Message Type ID
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

// Default Database location
//#define DBFILE       ("db/habpi.sqlite3")

/**
 * Database Class
 */
class Database {
public:
  // Database Constructor
  Database();

  // Database Destructor
  ~Database();

  // Connect to Database
  void connect(const char *dbFileName);

  // Disconnect from Database
  void disconnect();

  // Callback function for sqlite3_exec
  int callback(void *data, int argc, char **argv, char **azColName);

  // Insert data into Database
  void insertRecord(message_type_id_t messageTypeId, char *data);

  // Static Constants
  static const std::string DBFile;
  static const std::vector<std::string> MessageType;

private:
  sqlite3 *db;
  std::string dbName;
};
