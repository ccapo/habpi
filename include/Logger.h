/**
 * Logging Class
 *
 * Written By: Chris Capobianco
 * Date: 2017-08-02
 */
#pragma once

/**
 * Logger Class
 */
class Logger {
public:
	// Logger Constructor
	Logger();

	// Logger Destructor
	~Logger();

	// Startup Logger
	void startup(const char *rootLogFilename);

	// Shutdown Logger
	void shutdown();

	// Generic logging function
	void log(const char *tag, const char *message);

	// Debug logging function
	void debug(const char *message);

	// Info logging function
	void info(const char *message);

	// Notice logging function
	void notice(const char *message);

	// Alert logging function
	void alert(const char *message);

	// Warning logging function
	void warning(const char *message);

	// Error logging function
	void error(const char *message);

	// Static Constants
	static const std::string RootLogFile;
	static const std::string DebugTag;
	static const std::string InfoTag;
	static const std::string NoticeTag;
	static const std::string AlertTag;
	static const std::string WarningTag;
	static const std::string ErrorTag;

private:
	FILE *fout, *ferr;
};
