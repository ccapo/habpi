#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h> 

#define DEBUG 0

enum {
  RC_OK = 0,
  RC_ERROR_INPUT = -1,
  RC_ERROR_CONNECT = -2,
  RC_ERROR_DISCONNECT = -3,
  RC_ERROR_EXEC = -4
};

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
  MSG_CAM = 15
} message_type_id_t;

/* Message Type */
static char *message_type[16] = {
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

/* Random number in the range: 0.0 <= x < 1.0 */
float random_float() {
  return (float)rand()/(float)RAND_MAX;
}

/* Connect to sqlite3 DB */
int connectDB(sqlite3 **db, char *dbName) {
  int rc;

  /* Connect to database */
  rc = sqlite3_open(dbName, db);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "Can not open database: %s\n", sqlite3_errmsg(*db));
    return RC_ERROR_CONNECT;
  } else {
    fprintf(stderr, "Connected to database: %s\n", dbName);
    return RC_OK;
  }
}

/* Disconnect from sqlite3 DB */
int disconnectDB(sqlite3 *db, char *dbName) {
  int rc;

  /* Disconnect from database */
  rc = sqlite3_close(db);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "Can not close database: %s\n", sqlite3_errmsg(db));
    return RC_ERROR_DISCONNECT;
  } else {
    fprintf(stderr, "Disconnected from database: %s\n", dbName);
    return RC_OK;
  }
}

/* Callback function passed to sqlite3_exec */
static int callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  for(i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return RC_OK;
}

/* Insert data into sqlite3 DB */
int insertRecord(sqlite3 *db, message_type_id_t message_type_id, char *data) {
  int rc;
  char *zErrMsg = 0;
  char sql[128];

  /* Create SQL statement */
  sprintf(sql, "INSERT INTO message (message_type_id, message) VALUES (%d, '%s');", message_type_id, data);

  /* Start Transaction */
  rc = sqlite3_exec(db, "BEGIN", 0, 0, 0);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL1 error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return RC_ERROR_EXEC;
  } else {
    if(DEBUG) fprintf(stdout, "Started transaction\n");
  }

  /* Execute SQL statement */
  rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL2 error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return RC_ERROR_EXEC;
  } else {
    fprintf(stdout, "Inserted '%s' message record\n", message_type[message_type_id]);
  }

  /* End Transaction */
  rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL3 error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return RC_ERROR_EXEC;
  } else {
    if(DEBUG) fprintf(stdout, "Commited transaction\n");
  }
  
  return RC_OK;
}

int main(int argc, char *argv[]) {
  /* Define variables */
  sqlite3 *db;
  int rc;
  char data[32];
  char dbName[64];
  message_type_id_t message_type_id = MSG_NONE;

  /* Check for correct number of command line arguments */
  if(argc != 2) {
    fprintf(stderr, "Usage: %s dbfile\n", argv[0]);
    return RC_ERROR_INPUT;
  }

  /* Store dbName */
  strncpy(dbName, argv[1], 63);
  dbName[63] = '\0';

  /* Print debugging message */
  if(DEBUG) {
    fprintf(stderr, ">>> Debugging Mode Enabled <<<\n");
  }

  /* Connect to database */
  rc = connectDB(&db, dbName);
  if(rc != RC_OK) return rc;

  /* Loop indefinitely */
  while(1) {
    /* Randomly generate message type and message data */
    message_type_id = (message_type_id_t)(MSG_CAM*random_float() + MSG_SYS);
    sprintf(data, "%f", 100.0*random_float());
    
    /* Insert record into DB */
    rc = insertRecord(db, message_type_id, data);
    if(rc != RC_OK) break;

    /* Sleep for 1 sec */
    sleep(1);
  }

  /* Disconnect from database */
  rc = disconnectDB(db, dbName);
  if(rc != RC_OK) return rc;

  return RC_OK;
}
