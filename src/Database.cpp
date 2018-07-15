#include "HABPi.h"

// Database Constructor
Database::Database() {}

// Database Destructor
Database::~Database() {}

// Connect to Database
void Database::connect(const char *dbFileName) {
  int rc;
  char msg[Global::MaxLength];

  // Store DB filename
  dbName = std::string(dbFileName);

  // Connect to database
  rc = sqlite3_open(dbName.c_str(), &db);
  if(rc != SQLITE_OK) {
    sprintf(msg, "Can not open database: %s\n", sqlite3_errmsg(db));
    Module::logger.error(msg);
  } else {
    sprintf(msg, "Connected to database: %s", dbName.c_str());
    Module::logger.info(msg);
  }
}

// Disconnect from Database
void Database::disconnect() {
  int rc;
  char msg[Global::MaxLength];

  // Disconnect from database
  rc = sqlite3_close(db);
  if(rc != SQLITE_OK) {
    sprintf(msg, "Can not close database: %s\n", sqlite3_errmsg(db));
    Module::logger.error(msg);
  } else {
    sprintf(msg, "Disconnected from database: %s", dbName.c_str());
    Module::logger.info(msg);
  }
}

// Callback function passed to sqlite3_exec
int Database::callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  char msg[Global::MaxLength];

  for(i = 0; i < argc; i++) {
    sprintf(msg, "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    Module::logger.debug(msg);
  }
  Module::logger.debug("---");

  return Global::Ok;
}

// Insert data into sqlite3 DB
void Database::insertRecord(message_type_id_t messageTypeId, char *data) {
  int rc;
  char *zErrMsg = 0;
  char sql[Global::MaxLength], msg[Global::MaxLength];

  // Create SQL statement
  sprintf(sql, "INSERT INTO message (message_type_id, message) VALUES (%d, '%s');", messageTypeId, data);
  Module::logger.debug(sql);

  // Start Transaction
  rc = sqlite3_exec(db, "BEGIN", 0, 0, 0);
  if(rc != SQLITE_OK) {
    sprintf(msg, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    Module::logger.error(msg);
  } else {
    Module::logger.debug("Started transaction");
  }

  // Execute SQL statement
  rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if(rc != SQLITE_OK) {
    sprintf(msg, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    Module::logger.error(msg);
  } else {
    sprintf(msg, "Inserted %s message record", MessageType[messageTypeId].c_str());
    Module::logger.info(msg);
  }

  // End Transaction
  rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
  if(rc != SQLITE_OK) {
    sprintf(msg, "SQL error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    Module::logger.error(msg);
  } else {
    Module::logger.debug("Commited transaction");
  }
}

// Initialize static constants
const std::string Database::DBFile = "db/habpi.sqlite3";
const std::vector<std::string> Database::MessageType({
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
});