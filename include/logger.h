#ifndef LOGGER_H
#define LOGGER_H

/* Logging Tags */
#define DEBUG   "  [debug]"
#define INFO    "   [info]"
#define NOTICE  " [notice]"
#define WARNING "[warning]"
#define ERROR   "  [error]"

/* Messages will be appended to this file */
#define LOGFILE	"log/habpi.log"

/* Keeps track of whether the log file exists */
extern int log_file_exists;

/* Logging function */
void logger(const char *tag, const char *message);

#endif
