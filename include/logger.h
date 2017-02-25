#ifndef LOGGER_H
#define LOGGER_H

#define DEBUG   "  [debug]"
#define INFO    "   [info]"
#define NOTICE  " [notice]"
#define WARNING "[warning]"
#define ERROR   "  [error]"

// Messages will be appended to this file
#define LOGFILE	"log/habpi.log"

// Keeps track whether the log file is exists
extern int log_file_exists;

void logger(const char *tag, const char *message);

#endif
