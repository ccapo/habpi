#ifndef HABPI_H
#define HABPI_H

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
#include "sensors.h"

/* Debug Mode */
#define DEBUG_MODE 0

/* Version Control */
#define MAJOR_VERSION 0
#define MINOR_VERSION 2
#define REVISION_VERSION 0

/* Event loop delay [ms] */
#define DELAY 5000

/* Maximum length of DB filename */
#define STR_MAX 64

/* Return Codes */
enum {
  RC_OK = 0,
  RC_ERROR = 1
};

#endif
