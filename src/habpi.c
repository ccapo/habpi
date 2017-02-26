#include "habpi.h"
/*
 * HABPi Code
 *
 * Running on a Raspberry Pi Zero, this program communicates with
 * and collects data from sensors, a GPS and photos from a digital camera.
 *
 * The sensor data is recorded in a sqlite3 database, the photos are stored
 * in the images directory, and the program's activity is recorded in a log file.
 *
 * The GPS coordinates of the HAB, along with the sensor data and pictures
 * (depending on data transfer rate/distance from mission control) will be
 * broadcast using a long range radio.
 *
 * Author: Chris Capobianco
 * Date: 2017-02-22
 */
int main(int argc, char *argv[]) {
  sqlite3 *db;
  char dbName[STR_MAX], msg[DATA_STR_MAX];
  
  /* Check for correct number of command line arguments */
  if(argc != 2) {
    fprintf(stderr, "Usage: %s db.sqlite3\n", argv[0]);
    exit(RC_ERROR);
  }

  /* Store database name */
  strncpy(dbName, argv[1], STR_MAX - 1);
  dbName[STR_MAX - 1] = '\0';

  /* Print Program Header */
  logger(INFO, "============================");
  logger(INFO, "HABPi Code");
  sprintf(msg, "Version: %d.%d.%d", MAJOR_VERSION, MINOR_VERSION, REVISION_VERSION);
  logger(INFO, msg);
  logger(INFO, "Author: Chris Capobianco");
  logger(INFO, "============================");

  logger(NOTICE, "Start HABPi Program");
  printf("HABPi Program: Running...\n");

  /* Print debugging message */
  if(DEBUG_MODE) {
    logger(DEBUG, ">>> Debug Mode Enabled <<<");
  }

  logger(NOTICE, "Begin Component Initialization");

  /* Connect to database */
  connectDB(&db, dbName);

  /* Initialize WiringPi using GPIO pin convention */
  wiringPiSetupGpio();
  logger(INFO, "WiringPi Initialization Complete");

  /* Sensors Initialization */
  sensors_init();

  logger(NOTICE, "Finished Component Initialization");
  
  logger(NOTICE, "Begin HABPi Event Loop");

  /* Loop indefinitely */
  while(TRUE) {
    /* Update Sensors */
    sensors_update(db);

    /* Sleep for DELAY milliseconds */
    delay(DELAY);
  }
  
  logger(NOTICE, "End HABPi Event Loop");

  logger(NOTICE, "Begin Component Shutdown");

  /* Disconnect from database */
  disconnectDB(db, dbName);

  logger(NOTICE, "Finished Component Shutdown");

  logger(NOTICE, "Stopped HABPi Program");
  printf("HABPi Program: Stopped\n");

  return RC_OK;
}
