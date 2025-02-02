/**
 * Global Header
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <execinfo.h>

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <csignal>
#include <thread>
#include <atomic>
#include <algorithm> 
#include <cctype>
#include <locale>

#include <wiringPi.h>
#include <sqlite3.h>

class Global {
  public:
  Global() {}
  ~Global() {}

  // Debug Flag
  static const bool Debug = false;

  // Return Codes
  static const int Ok = 0;
  static const int Error = 1;

  // Version Control
  static const int Major = 0;
  static const int Minor = 9;
  static const int Revision = 3;

  // Max. String Length
  static const int MaxLength = 128;

	// trim from start (in place)
	static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
      return !std::isspace(ch);
    }));
	}

	// trim from end (in place)
	static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return !std::isspace(ch);
    }).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
	}

	// trim from start (copying)
	static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
	}

	// trim from end (copying)
	static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
	}

	// trim from both ends (copying)
	static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
	}

  static std::string exec(const char *cmd) {
		char buffer[128];
		std::string result = "";
		FILE *pipe = popen(cmd, "r");
		if (!pipe)
			throw std::runtime_error("popen() failed!");
		try {
			while (!feof(pipe)) {
				if (fgets(buffer, 128, pipe) != NULL) {
					result += buffer;
				}
			}
		} catch (...) {
			pclose(pipe);
			throw;
		}
		pclose(pipe);
		return trim_copy(result);
  }
};