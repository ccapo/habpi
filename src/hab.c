/*
 * Hab Code
 *
 * Using a Raspberry Pi, and collecting data from
 * a BMP180 temperature and pressure sensor,
 * and writing results into a sqlite3 database.
 *
 * Author: Chris Capobianco
 * Date: 2017-02-22
 * Version: 0.1.0
 */

#include "hab.h"

int main(int argc, char *argv[]) {
  /* Declare variables */
  int rc;
  float t, p, h;
  sqlite3 *db;
  char data[DATA_STR_MAX], dbName[DB_STR_MAX], msg[128];
  message_type_id_t message_type_id = MSG_NONE;
  
  /* Check for correct number of command line arguments */
  if(argc != 2) {
    fprintf(stderr, "Usage: %s dbfile\n", argv[0]);
    exit(RC_ERROR_ARG);
  }

  /* Store database name */
  strncpy(dbName, argv[1], DB_STR_MAX - 1);
  dbName[DB_STR_MAX - 1] = '\0';

  /* Print Program Header */
  logger(INFO, "============================");
  logger(INFO, "| HAB Code                 |");
  logger(INFO, "|                          |");
  sprintf(msg, "| Version: %d.%d.%d           |", MAJOR_VERSION, MINOR_VERSION, REVISION_VERSION);
  logger(INFO, msg);
  logger(INFO, "| Author: Chris Capobianco |");
  logger(INFO, "| Date: 2017-02-22         |");
  logger(INFO, "============================");
  logger(INFO, "Payload Components:");
  logger(INFO, " + Raspberry Pi 3 B");
  //logger(INFO, " + Raspberry Pi Zero and Camera");
  logger(INFO, " + Temperature and Pressure Sensor (BMP180)");
  //logger(INFO, " + Temperature and Pressure Sensor (MPL3115A2)");
  //logger(INFO, " + Magnetometer Sensor (HMC5883L)");
  //logger(INFO, " + GPS Sensor (Adafruit Ultimate GPS)");
  //logger(INFO, " + Radio Transceiveer (XBP9B-XCUT-002)");

  logger(NOTICE, "HAB Program: [Begin]");
  printf("HAB Program: Running...\n");

  /* Print debugging message */
  if(DEBUG) {
    logger("debug", ">>> Debugging Mode Enabled <<<");
  }

  logger(NOTICE, "Component Initialization: [Start]");

  /* Connect to database */
  connectDB(&db, dbName);

  /* Setup wiringPi using GPIO pin convention */
  wiringPiSetupGpio();
  logger(INFO, "Wiring Pi Setup Complete");

  /* Setup BMP180 sensor */
  rc = bmp180Setup(BMP180_PINBASE);
  if(rc != TRUE) exit(rc);
  logger(INFO, "BMP180 Sensor Setup Complete");

  logger(NOTICE, "Component Initialization: [Complete]");
  
  logger(NOTICE, "HAB Event Loop: [Begin]");

  /* Loop indefinitely */
  while(TRUE) {
    /* Read temperature, pressure and compute altitude */
    t = analogRead(BMP180_PINBASE)/10.0;
    p = analogRead(BMP180_PINBASE + 1)/10.0;
    h = ALPHAINV*(1.0 - pow(p/P0, BETAINV))/100.0;

    //sprint(msg, "Temp = %f C", t);
    //logger(INFO, msg);
    //sprint(msg, "Pres = %f hPa", p);
    //logger(INFO, msg);
    //sprint(msg, "Alt = %f m", h);
    //logger(INFO, msg);

    /* Set message type and message data */
    message_type_id = MSG_TEMP;
    sprintf(data, "%f", t);

    /* Insert record into DB */
    insertRecord(db, message_type_id, data);

    /* Set message type and message data */
    message_type_id = MSG_BARO;
    sprintf(data, "%f", p);

    /* Insert record into DB */
    insertRecord(db, message_type_id, data);

    /* Set message type and message data */
    message_type_id = MSG_BARO_ALT;
    sprintf(data, "%f", h);

    /* Insert record into DB */
    insertRecord(db, message_type_id, data);

    /* Sleep for DELAY milliseconds */
    delay(DELAY);
  }
  
  logger(NOTICE, "HAB Event Loop: [End]");

  logger(NOTICE, "Component Shutdown: [Start]");

  /* Disconnect from database */
  disconnectDB(db, dbName);

  logger(NOTICE, "Component Shutdown: [Complete]");

  logger(NOTICE, "HAB Program: [End]");
  printf("HAB Program: Stopped\n");

  return RC_OK;
}
