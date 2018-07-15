/**
 * HABPi Code
 *
 * Running on a Raspberry Pi Zero, this program communicates with
 * and collects data from sensors, a GPS and photos from a digital camera.
 *
 * The sensor data is recorded in a sqlite3 database, the photos are stored
 * in the images directory, and the program's activity is recorded in a log file.
 *
 * The GPS coordinates of the HAB, along with the sensor data and pictures
 * (depending on data transfer rate/distance from mission control) will be
 * broadcast using a long range radio.
 *
 * Author: Chris Capobianco
 * Date: 2017-02-22
 */
#include "HABPi.h"

// Signal Handler
void signalHandler(int signum) {
  void *array[10];
  size_t size;

  Module::isRunning = false;
  std::cerr << "Interrupt signal (" << signum << ") received" << std::endl;

  // Print stack trace in event of segmentation fault
  if (signum == SIGSEGV) {
    // Get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // Print out all the frames to stderr
    std::cerr << "Error: signal " << signum << std::endl;
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
  }
}

// Reset GPSD
void resetGPSD() {
  std::cout << "GPSD: Reset Begin" << std::endl;
  system("sudo killall gpsd > /dev/null 2>&1");
  system("sleep 1");
  system("sudo gpsd /dev/ttyS0 -F /var/run/gpsd.sock");
  system("sleep 1");
  std::cout << "GPSD: Reset End" << std::endl;
}

int main(int argc, char *argv[]) {
  std::atomic<bool> sensorReady(false);
  std::atomic<bool> imageReady(false);
  char dbFileName[Global::MaxLength];

  // Trigger execution of signalHandler if we receive these interrupts
  signal(SIGHUP, signalHandler);
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGPIPE, signalHandler);
  signal(SIGSEGV, signalHandler);

  // Disable buffering to STDOUT
  std::cout.setf(std::ios::unitbuf);
  std::cerr.setf(std::ios::unitbuf);

  // Reset GPSD
  resetGPSD();

  // Create Module Instance
  Module module;

  // Print Program Header
  std::cout << "+=~=~=~=~=~=~=~=~=~=~=~=~=~=~=+" << std::endl;
  std::cout << "| HABPi Code                  |" << std::endl;
  std::cout << "| Version: " << Global::Major << "." << Global::Minor << "." << Global::Revision << "              |" << std::endl;
  std::cout << "| Author: Chris Capobianco    |" << std::endl;
  std::cout << "+=~=~=~=~=~=~=~=~=~=~=~=~=~=~=+" << std::endl;

  // Create Logger and set database filename
  if (argc == 1) {
    // Set Logger files using default log location
    Module::logger.startup(Logger::RootLogFile.c_str());

    // Store database filename
    strncpy(dbFileName, Database::DBFile.c_str(), Global::MaxLength - 1);
    dbFileName[Global::MaxLength - 1] = '\0';
  } else if (argc == 3) {
    // Set Logger files using custom log location
    Module::logger.startup(argv[1]);

    // Store database filename
    strncpy(dbFileName, argv[2], Global::MaxLength - 1);
    dbFileName[Global::MaxLength - 1] = '\0';
  } else {
    // Abort if incorrect number of arguments are provided
    std::cerr << "Usage: " << argv[0] << " [/path/to/log/rootfilename] [/path/to/db.sqlite3]" << std::endl;
    exit(Global::Error);
  }

  Module::logger.notice("Starting HABPi Program");

  if (Global::Debug) std::cout << "Debug Mode" << std::endl;

  // Print debugging message
  Module::logger.debug(">>> Debug Mode Enabled <<<");

  Module::logger.notice("Begin Component Initialization");

  // Initialize WiringPi using default pin convention
  wiringPiSetup();
  Module::logger.info("WiringPi Initialization Complete");

  // Module Initialization
  module.startup(dbFileName);

  Module::logger.notice("Finished Component Initialization");
  
  Module::logger.notice("Begin HABPi Event Loop");

  // Launch a thread for sensorUpdate
  std::thread sensorThread(&module.sensorUpdate, std::ref(sensorReady));
  
  // Launch a thread for cameraUpdate
  std::thread cameraThread(&module.cameraUpdate, std::ref(imageReady));

  // Module Broadcast Update
  module.broadcastUpdate(sensorReady, imageReady);

  // Join the thread with the main thread
  sensorThread.join();
  
  // Join the thread with the main thread
  cameraThread.join();
  
  Module::logger.notice("End HABPi Event Loop");

  Module::logger.notice("Begin Component Shutdown");

  // Module Shutdown
  module.shutdown();

  Module::logger.notice("Finished Component Shutdown");

  Module::logger.notice("Stopping HABPi Program");

  // Logger Shutdown
  Module::logger.shutdown();

  std::cout << "Sensor Messages Sent: " << Module::sensorCounter << std::endl;
  std::cout << "Image Messages Sent:  " << Module::imageCounter*Serializer::NChunks << std::endl;
  std::cout << "Sensor Messages ACK:  " << Module::sensorAckCounter << std::endl;
  std::cout << "Image Messages ACK:   " << Module::imageAckCounter << std::endl;
  std::cout << "Sensor Messages NAK:  " << Module::sensorNakCounter << std::endl;
  std::cout << "Image Messages NAK:   " << Module::imageNakCounter << std::endl;

  return Global::Ok;
}
