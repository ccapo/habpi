#include "HABPi.h"

// Logger Constructor
Logger::Logger(): fout(NULL), ferr(NULL) {}

// Logger Destructor
Logger::~Logger() {}

// Startup Logger
void Logger::startup(const char *rootLogFilename) {
  // Disable buffering to STDOUT
  std::cout.setf(std::ios::unitbuf);
  std::cerr.setf(std::ios::unitbuf);

  std::ostringstream outstream, errstream;
  outstream << rootLogFilename << ".stdout";
  errstream << rootLogFilename << ".stderr";

  std::string logPath = outstream.str();
  std::string errPath = errstream.str();

  // Open new log file, or append to existing log file
  fout = fopen(logPath.c_str(), "a");
  std::cout << "Opened log file " << logPath << std::endl;

  // Open new error file, or append to existing log file
  ferr = fopen(errPath.c_str(), "a");
  std::cout << "Opened error file " << errPath << std::endl;
}

// Shutdown Logger
void Logger::shutdown() {
  if (fout != NULL) {
    // Close log file
    fclose(fout);
    std::cout << "Closed log file" << std::endl;
  } else {
    std::cerr << "Unable to close log file" << std::endl;
  }

  if (ferr != NULL) {
    // Close error file
    fclose(ferr);
    std::cout << "Closed error file" << std::endl;
  } else {
    std::cerr << "Unable to close error file" << std::endl;
  }
}

// Generic logging function
void Logger::log(const char *tag, const char *message) {
  time_t now;
  struct tm *tm;
  char timestamp[Global::MaxLength], msg[Global::MaxLength];

  // Create timestamp string
  time(&now);
  tm = gmtime(&now);
  sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

  // Create logging record with timestamp, tag and message content
  sprintf(msg, "%s %s: %s\n", timestamp, tag, message);

  // If the log files are open, write to log file
  if((fout != NULL) && (ferr != NULL)) {
    // If the ERROR tag is passed, send to error file, otherwise send to log file
    if(strcasecmp(tag, ErrorTag.c_str()) != 0) {
      fputs(msg, fout);
    } else {
      fputs(msg, ferr);
    }
  } else {
    // Fallback to local filesystem
    // If the ERROR tag is passed, send to stderr, otherwise send to stdout
    if(strcasecmp(tag, ErrorTag.c_str()) != 0) {
      std::cout << msg << std::endl;
    } else {
      std::cerr << msg << std::endl;
    }
  }
}

// Debug logging function
void Logger::debug(const char *message) {
  if (Global::Debug) Logger::log(DebugTag.c_str(), message);
}

// Info logging function
void Logger::info(const char *message) {
  Logger::log(InfoTag.c_str(), message);
}

// Notice logging function
void Logger::notice(const char *message) {
  Logger::log(NoticeTag.c_str(), message);
}

// Alert logging function
void Logger::alert(const char *message) {
  Logger::log(AlertTag.c_str(), message);
}

// Warning logging function
void Logger::warning(const char *message) {
  Logger::log(WarningTag.c_str(), message);
}

// Error logging function
void Logger::error(const char *message) {
  Logger::log(ErrorTag.c_str(), message);
}

// Initialize static constants
const std::string Logger::RootLogFile = "log/habpi";
const std::string Logger::DebugTag = "[debug]";
const std::string Logger::InfoTag = "[info]";
const std::string Logger::NoticeTag = "[notice]";
const std::string Logger::AlertTag = "[alert]";
const std::string Logger::WarningTag = "[warning]";
const std::string Logger::ErrorTag = "[error]";