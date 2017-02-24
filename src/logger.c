#include "hab.h"

int log_file_exists = 0;

void logger(const char *tag, const char *message) {
  FILE *fp;
  time_t now;
  struct tm *tm;
  char timestamp[32], msg[128];

  time(&now);

  if(log_file_exists == 0) {
    log_file_exists = 1;
    fp = fopen(LOGFILE, "w");
  } else {
    fp = fopen(LOGFILE, "a");
  }

  if(fp == NULL) {
    if(log_file_exists == 1) {
      log_file_exists = 0;
    }
    return;
  } else {
    time(&now);
    tm = gmtime(&now);
    sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    sprintf(msg, "%s %s: %s\n", timestamp, tag, message);
    fputs(msg, fp);
    fclose(fp);
    if(strcasecmp(tag, ERROR) == 0) exit(1);
    return;
  }

  if(fp) {
    fclose(fp);
    return;
  }
}
