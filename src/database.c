#include "habpi.h"

/* Message Type */
static char *message_type[NMSG] = {
  "None",
  "System Status",
  "GPS Latitude",
  "GPS Longitude",
  "GPS Altitude",
  "Temperature",
  "Pressure",
  "Pressure Altitude",
  "Magnetic X Component",
  "Magnetic Y Component",
  "Magnetic Z Component",
  "Magnetic Pitch Angle",
  "Magnetic Roll Angle",
  "Magnetic Heading Angle",
  "Battery Status",
  "Camera Status"
};

/* Connect to sqlite3 DB */
void connectDB(sqlite3 **db, char *dbName) {
  int rc;
  char msg[DATA_STR_MAX];

  /* Connect to database */
  rc = sqlite3_open(dbName, db);
  if(rc != SQLITE_OK) {
    sprintf(msg, "Can not open database: %s", sqlite3_errmsg(*db));
    logger(ERROR, msg);
  } else {
    sprintf(msg, "Connected to database: %s", dbName);
    logger(INFO, msg);
  }
}

/* Disconnect from sqlite3 DB */
void disconnectDB(sqlite3 *db, char *dbName) {
  int rc;
  char msg[DATA_STR_MAX];

  /* Disconnect from database */
  rc = sqlite3_close(db);
  if(rc != SQLITE_OK) {
    sprintf(msg, "Can not close database: %s", sqlite3_errmsg(db));
    logger(ERROR, msg);
  } else {
    sprintf(msg, "Disconnected from database: %s", dbName);
    logger(INFO, msg);
  }
}

/* Callback function passed to sqlite3_exec */
int callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  char msg[DATA_STR_MAX];

  for(i = 0; i < argc; i++) {
    sprintf(msg, "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    logger(INFO, msg);
  }
  logger(INFO, "---");

  return RC_OK;
}

/* Insert data into sqlite3 DB */
void insertRecord(sqlite3 *db, message_type_id_t message_type_id, char *data) {
  int rc;
  char *zErrMsg = 0;
  char sql[SQL_STR_MAX], msg[DATA_STR_MAX];

  /* Create SQL statement */
  sprintf(sql, "INSERT INTO message (message_type_id, message) VALUES (%d, '%s');", message_type_id, data);
  if(DEBUG_MODE) logger(DEBUG, sql);

  /* Start Transaction */
  rc = sqlite3_exec(db, "BEGIN", 0, 0, 0);
  if(rc != SQLITE_OK) {
    sprintf(msg, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    logger(ERROR, msg);
  } else {
    if(DEBUG_MODE) logger(DEBUG, "Started transaction");
  }

  /* Execute SQL statement */
  rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
  if(rc != SQLITE_OK) {
    sprintf(msg, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    logger(ERROR, msg);
  } else {
    sprintf(msg, "Inserted %s message record", message_type[message_type_id]);
    logger(INFO, msg);
  }

  /* End Transaction */
  rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
  if(rc != SQLITE_OK) {
    sprintf(msg, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    logger(ERROR, msg);
  } else {
    if(DEBUG_MODE) logger(DEBUG, "Commited transaction");
  }
} 
