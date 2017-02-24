#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sqlite3.h>
#include <wiringPi.h>
#include <bmp180.h>
#include "logger.h"
#include "database.h"

/* Debug Flag */
#define DEBUG_MODE 0

/* Version Control */
#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define REVISION_VERSION 1

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

/* Event loop delay [ms] */
#define DELAY 5000

/* Maximum length of DB filename */
#define DB_STR_MAX 64

/* Maximum length of data string in DB */
#define DATA_STR_MAX 32

/* Return Codes */
enum {
  RC_OK = 0,
  RC_ERROR_ARG = 1,
  RC_ERROR_DB_CONNECT = 2,
  RC_ERROR_DB_DISCONNECT = 3,
  RC_ERROR_DB_EXEC = 4
};
