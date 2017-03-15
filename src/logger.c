#include "habpi.h"

int log_file_exists = 0;

void logger(const char *tag, const char *message) {
  FILE *fp;
  time_t now;
  struct tm *tm;
  char timestamp[DATA_STR_MAX], msg[SQL_STR_MAX];

  /* Check if log file already exists, and either open new or append to existing */
  if(log_file_exists == 0) {
    log_file_exists = 1;
    fp = fopen(LOGFILE, "w");
  } else {
    fp = fopen(LOGFILE, "a");
  }

  /* If file did not open, reset status flag and return */
  if(fp == NULL) {
    if(log_file_exists == 1) {
      log_file_exists = 0;
    }
    return;
  } else {
    /* Create timestamp string */
    time(&now);
    tm = gmtime(&now);
    sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    /* Create logging record with timestamp, tag and message content */
    sprintf(msg, "%s - %s: %s\n", timestamp, tag, message);
    fputs(msg, fp);
    
    /* Close log file */
    fclose(fp);
    
    /* If the ERROR tag is passed, abort the program */
    if(strcasecmp(tag, ERROR) == 0) {
      fprintf(stderr, "Program Aborted\n");
      exit(RC_ERROR);
    }

    return;
  }

  /* If log file is still open, close it */
  if(fp) {
    fclose(fp);
    return;
  }
}
