#pragma once

#include "Global.h"
#include "i2c_bus.h"
#include "spi_bus.h"
#include "Database.h"
#include "Serializer.h"
#include "DHT_U.h"
#include "FXAS21002C.h"
#include "FXOS8700.h"
#include "MPL3115A2_U.h"
#include "Madgwick.h"
#include "Mahony.h"
#include "libgpsmm.h"
#include "Camera.h"

/**
 * Module Class
 */
class Module {
public:
	// Module Constructor
	Module();

	// Module Destructor
	~Module();

  // Module Startup
	void startup(const char *dbFileName);

	// Module Shutdown
	void shutdown();

	// Module Broadcast Update
	void broadcastUpdate(std::atomic<bool> &sensorReady, std::atomic<bool> &imageReady);

	// Send SPI Command
	bool sendSPICommand(uint8_t command, uint8_t len, uint8_t *rxData);

	// Static variables

	// Flags to enable sensor component
  static bool isRunning, enableGPS, enableAHRS, enableMPL, enableDHT, enableIMG, recordVideo;

  // Image Number and Chunk Number
  static int imageNumber, imageChunkNumber;

  // Video Number
  static int videoNumber;

  // Sensor, Image and Battery Messages
  static sensor_msg_t sensorMsg;
  static image_msg_t imageMsg;
  static battery_msg_t batteryMsg;

  // Image broadcast queue
  static std::vector<image_msg_t> broadcast_queue;

  // Sensor and Image Payloads
  static uint8_t sensorPayload[Serializer::SensorSize], imagePayload[Serializer::ImageSize];

	// Serializer
	static Serializer serializer;

	// Logger
	static Logger logger;

  // Sensors
  static GPS gps;
  static AHRS ahrs;
  static MPL3115A2_Unified mpl;
  static DHT_Unified dht;
  static Camera camera;

  // Debugging counters
  static int sensorCounter, imageCounter;
  static int sensorAckCounter, imageAckCounter;
  static int sensorNakCounter, imageNakCounter;

  // Static Constants

  // Paths to I2C and SPI devices
  static const std::string I2CPath;
  static const std::string SPIPath;

  // Timing Constants
	static const int Microsecond = 1000000;
	static const int SensorDelay = Microsecond;
	static const int ImageDelay = 100 * Microsecond;
	static const int BroadcastDelay = 9 * Microsecond / 100;
	static const int SpiTimeout = 10 * Microsecond;
	static const int MinDelay = 10;

	// SPI Messages
	static const uint8_t Nul = 0x00;
	static const uint8_t Stx = 0x02;
	static const uint8_t Eot = 0x04;
	static const uint8_t Enq = 0x05;
	static const uint8_t Ack = 0x06;
	static const uint8_t Nak = 0x15;
	static const uint8_t Us = 0x1F;

	// SPI Commands
	static const uint8_t SensorCmd = 0x60;
	static const uint8_t ImageCmd = 0x70;
	static const uint8_t BatteryCmd = 0x90;

	// Pressure-Altitude Coefficient
	static const constexpr double Alpha = 2.25577E-7;

	// Pressure-Altitude Exponent
	static const constexpr double Beta = 5.25588;

	// Inverse Pressure-Altitude Coefficient
	static const constexpr double AlphaInv = 1.0/Alpha;

	// Pressure-Altitude Exponent
	static const constexpr double BetaInv = 1.0/Beta;

	// Pressure at Sea Level [hPa]
	static const constexpr double P_0 = 1013.25;

  // Static Methods

	// Module update
	static void update();

	// Module Sensor Update
	static void sensorUpdate(std::atomic<bool> &sensorReady);

	// Module Camera Update
	static void cameraUpdate(std::atomic<bool> &imageReady);

	// Digi XBee API Frame Packet Checksum
  static uint8_t checksum(uint8_t len, uint8_t *buffer);

private:
	// SPI Parameters
  spi_bus spi;

	// I2C bus
  i2c_bus i2c;

  // Database Declaration
	Database database;
};
