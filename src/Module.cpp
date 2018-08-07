#include "HABPi.h"

// Module Constructor
Module::Module() {}

// Module Destructor
Module::~Module() {}

// Module Startup
void Module::startup(const char *dbFileName) {
  // Open Connection to I2C bus
  i2c.open(I2CPath);

  // Open Connection to SPI bus
  spi.open(SPIPath);

  // Connect to Database
  database.connect(dbFileName);

  // Initialize GPS
  //enableGPS = gps.begin();

  // Initialize Orientation Sensor
  //enableAHRS = ahrs.begin(i2c);
  
  // Initialize Temperature and Pressure Sensor Sensor
  enableMPL = mpl.begin(i2c);

  // Initialize Temperature and Humidity Sensor
  //enableDHT = dht.begin();
  
  // Initialize Camera
  enableIMG = camera.begin();

  // Set message types
  sensorMsg.type = SensorCmd;
  imageMsg.type = ImageCmd;

  logger.notice("Sensor Startup Complete");
}

// Module Shutdown
void Module::shutdown() {
  // Close I2C connection
  i2c.close();

  // Close SPI Device
  spi.close();

  // Disconnect from Database
  database.disconnect();
}

void Module::broadcastUpdate(std::atomic<bool> &sensorReady, std::atomic<bool> &imageReady) {
  // Main broadcast loop
  bool receiveStatus = false;
  bool cmdStatus = false;
  uint8_t response[Serializer::PayloadSize] = {0};
  std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

  // Initialize battery voltages
  batteryMsg.bat_rpi = 0.0;
  batteryMsg.bat_ard = 0.0;

  while (isRunning == true) {
    currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count() >= BroadcastDelay) {
      prevTime = currentTime;
      cmdStatus = sendSPICommand(BatteryCmd, Serializer::BatterySize, response);
      if (cmdStatus == true) {
        // Deserialize battery message
        serializer.deserialize(response, &batteryMsg);

        // Compute message checksum, and compare with receieved checksum
        uint8_t chksum = checksum(Serializer::BatterySize - 1, response);
        if (chksum == response[Serializer::BatterySize - 1]) {
          // Print battery voltages
          if (receiveStatus == false) {
            receiveStatus = true;
            //std::cout << "RPi Battery: " << batteryMsg.bat_rpi << " V, ";
            //std::cout << "Ard Battery: " << batteryMsg.bat_ard << " V" << std::endl;
            std::cout << "Receieved Battery Voltages" << std::endl;
          }
        } else {
          // Received checksum does not match computed checksum
          std::cerr << "Checksum for battery voltages does not match computed checksum:";
          std::cerr << " 0x" << std::hex << static_cast<uint16_t>(chksum);
          std::cerr << " != 0x" << std::hex << static_cast<uint16_t>(response[Serializer::BatterySize]) << std::dec << std::endl;
        }
      } else {
        // Something went wrong
        logger.error("Unable to retrieve battery voltages");
      }
      usleep(MinDelay);

      // If we have sensor data, then send to Arduino
      if (sensorReady == true) {
        cmdStatus = sendSPICommand(SensorCmd, Serializer::SensorSize, response);
        if (cmdStatus == true) {
          sensorAckCounter++;
          std::cout << "Sent sensor data successfully" << std::endl;
        } else {
          // Something went wrong
          sensorNakCounter++;
          logger.error("Unable to send sensor data");
          std::cerr << "Unable to send sensor data" << std::endl;
        }

        receiveStatus = false;
        sensorReady = false;
      } else if (imageReady == true) {
        // Else if we have image data, then send to Arduino
        cmdStatus = sendSPICommand(ImageCmd, Serializer::ImageSize, response);
        if (cmdStatus == true) {
          imageAckCounter++;
          std::cout << "Sent Image Data: " << imageChunkNumber << std::endl;
        } else {
          // Something went wrong
          imageNakCounter++;
          logger.error("Unable to send image data");
          std::cerr << "Unable to send image data" << std::endl;
        };

        // Check if broadcast queue is empty
        if (!broadcast_queue.empty()) {
          // Increment image chunk counter
          imageChunkNumber++;

          // Load next image message from front of broadcast queue
          imageMsg = broadcast_queue[0];
          broadcast_queue.erase(broadcast_queue.begin());

          // Clear imagePayload message
          std::memset(imagePayload, 0, Serializer::ImageSize);

          // Serialize image message
          serializer.serialize(&imageMsg, imagePayload);

          // Compute and store checksum for imagePayload message
          uint8_t chksum = checksum(Serializer::ImageSize - 1, imagePayload);
          //std::cout << "Sent Checksum: 0x" << std::hex << static_cast<uint16_t>(chksum) << std::dec << std::endl;
          imagePayload[Serializer::ImageSize - 1] = chksum;
        } else {
          // Reset chunk counter and set imageReady to false
          imageChunkNumber = 1;
          imageReady = false;
        }

        receiveStatus = false;
      }
      //usleep(MinDelay);
    }
  }
}

/**
 * sendSPICommand
 *
 * A protocol that uses the spi.transferByte and spi.transferByteArray
 * functions to send a command and packet to the Arduino, as well
 * as capturing the response.
 */
bool Module::sendSPICommand(uint8_t command, uint8_t len, uint8_t *rxData) {
  uint8_t response = Nul;
  bool ready = false;

  memset(rxData, 0, len);

  // An initial handshake sequence sends a one byte start code
  // (STX) and loops at most 4096 times until it receives the
  // acknowledgment code (ACK) and sets the ready flag to true.
  for (int j = 0; j < 4096; j++) {
    // Send STX byte to initiate communication
    spi.transferByte(Stx);
    usleep(MinDelay);

    // Send command byte
    spi.transferByte(command);
    usleep(MinDelay);

    // Send ENQ message to get response from command message
    response = spi.transferByte(Enq);
    if (response == Ack) {
      ready = true;
      break;
    }
    usleep(MinDelay);
  }

  // If we are ready to continue, otherwise wait
  if (ready == true) {
    switch (command) {
      case BatteryCmd: {
        for (uint8_t i = 0; i < len; i++) {
          // Send ENQ message, and store response
          rxData[i] = spi.transferByte(Enq);
          usleep(MinDelay);
        }
      } break;
      case SensorCmd: {
        // Send sensor payload
        for (uint8_t i = 0; i < len; i++) {
          response = spi.transferByte(sensorPayload[i]);
          usleep(MinDelay);
        }

        // Send ENQ message to get response from last message
        response = spi.transferByte(Enq);
        usleep(MinDelay);
        if (response != Ack) {
          std::cerr << "Did not receive ACK for sensor message:";
          std::cerr << " 0x" << std::hex << static_cast<uint16_t>(response) << std::endl;
          return false;
        }
      } break;
      case ImageCmd: {
        // Send image payload
        for (uint8_t i = 0; i < len; i++) {
          response = spi.transferByte(imagePayload[i]);
          usleep(MinDelay);
        }

        // Send ENQ message to get response from last message
        response = spi.transferByte(Enq);
        usleep(MinDelay);
        if (response != Ack) {
          std::cerr << "Did not receive ACK for image message:";
          std::cerr << " 0x" << std::hex << static_cast<uint16_t>(response) << std::endl;
          return false;
        }
      } break;
      default: {
        std::cerr << "Unrecognized command: 0x" << std::hex << static_cast<uint16_t>(command) << std::endl;
        return false;
      } break;
    }
    return true;
  } else {
    // Failed to initiate SPI handshake
    logger.error("SPI Handshake Failure");
    return false;
  }
}

// Module Update
void Module::update() {
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  // Update GPS Sensor
  if(enableGPS) {
    // Store updated GPS values
    gps.update();
  }

  // Update AHRS Sensor
  if(enableAHRS) {
    // Store updated AHRS values
    float roll, pitch, heading;
    ahrs.update(roll, pitch, heading);
    sensorMsg.ahrs_head = heading;
    sensorMsg.ahrs_pitch = pitch;
    sensorMsg.ahrs_roll = roll;
  }

  // Update MPL3115A2 Sensor
  if(enableMPL) {
    // Store updated MPL3115A2 values
    float temperature, pressure, altitude;
    mpl.update(temperature, pressure, altitude);
    sensorMsg.mpl_temp = temperature;
    sensorMsg.mpl_pres = pressure;
    sensorMsg.mpl_alt = altitude;
  }
  
  // Update DHT11 Sensor
  if(enableDHT) {
    // Store updated DHT11 values
    float temperature, relative_humidity;
    dht.update(temperature, relative_humidity);
    sensorMsg.dht_temp = temperature;
    sensorMsg.dht_relh = relative_humidity;
  }

  // Update battery voltage
  sensorMsg.bat_rpi = batteryMsg.bat_rpi;
  sensorMsg.bat_ard = batteryMsg.bat_ard;

  // Insert sensor values into database

  std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
  int diff = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
  std::cout << "Senor Update Timing (s): " << static_cast<double>(diff)/static_cast<double>(Microsecond) << std::endl;
}

// This function will be called from a thread
void Module::sensorUpdate(std::atomic<bool> &sensorReady) {
  std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

  while (isRunning == true) {
    currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count() >= SensorDelay) {
      // Update Module
      update();

      // Add logic to set recordVideo flag
      // if (sensorMsg.gps_alt <= 500.0) {
      //   recordVideo = true;
      // }

      // Add logic to set proximityFlag flag
      // sensorMsg.proximityFlag = Nul;
      // if (sensorMsg.gps_alt < 250.0 && sensorMsg.gps_cli < 0.0) {
      //   sensorMsg.proximityFlag = Us;
      // }

      // Clear sensorPayload message
      std::memset(sensorPayload, 0, Serializer::SensorSize);

      // Serialize sensor message
      serializer.serialize(&sensorMsg, sensorPayload);

      if (Global::Debug) serializer.print(sensorMsg);

      // Compute and store checksum for sensorPayload message
      uint8_t chksum = checksum(Serializer::SensorSize - 1, sensorPayload);
      //std::cout << "Sent Checksum: 0x" << std::hex << static_cast<uint16_t>(chksum) << std::dec << std::endl;
      sensorPayload[Serializer::SensorSize - 1] = chksum;

      prevTime = currentTime;
      std::cout << "sensorLoop: " << ++sensorCounter << std::endl;
      sensorReady = true;
    }
  }
}

// This function will be called from a thread
void Module::cameraUpdate(std::atomic<bool> &imageReady) {
  std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

  while (isRunning == true) {
    if (imageReady == false) {
      currentTime = std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count() >= ImageDelay) {
        if(enableIMG) {
          // Set camera mode
          uint8_t mode = Camera::ImageMode;
          if (recordVideo == true) {
            recordVideo = false;
            mode = Camera::VideoMode;
          }

          // Update Camera
          camera.update(mode);

          // Construct image message if in ImageMode
          if (mode == Camera::ImageMode) {
            // Load thumbnail image from disk, and partition into NChunks
            camera.load();

            // Populate image message with first image message from broadcast queue
            imageMsg = broadcast_queue[0];
            broadcast_queue.erase(broadcast_queue.begin());

            // Clear imagePayload message
            std::memset(imagePayload, 0, Serializer::ImageSize);

            // Serialize image message
            serializer.serialize(&imageMsg, imagePayload);

            // Compute and store checksum for imagePayload message
            uint8_t chksum = checksum(Serializer::ImageSize - 1, imagePayload);
            //std::cout << "Sent Checksum: 0x" << std::hex << static_cast<uint16_t>(chksum) << std::dec << std::endl;
            imagePayload[Serializer::ImageSize - 1] = chksum;

            prevTime = currentTime;
            std::cout << "imageLoop: " << ++imageCounter << std::endl;
            imageReady = true;
          }
        }
      }
    }
  }
}

// Digi XBee API Frame Packet Checksum
// See http://knowledge.digi.com/articles/Knowledge_Base_Article/Calculating-the-Checksum-of-an-API-Packet
uint8_t Module::checksum(uint8_t len, uint8_t *buffer) {
  uint8_t check = 0;

  for (uint8_t i = 0; i < len; i++) {
    check += buffer[i];
  }

  return 0xFF - (check & 0xFF);
}

// Initialize static constants
const std::string Module::I2CPath = "/dev/i2c-1";
const std::string Module::SPIPath = "/dev/spidev0.0";

// Initialize static variables
int Module::imageNumber = 1000000;
int Module::imageChunkNumber = 1;
int Module::videoNumber = 1000000;
bool Module::isRunning = true;
bool Module::enableGPS = false;
bool Module::enableAHRS = false;
bool Module::enableMPL = true;
bool Module::enableDHT = false;
bool Module::enableIMG = true;
bool Module::recordVideo = false;
uint8_t Module::sensorPayload[Serializer::SensorSize] = {0}; //"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut eu volutpat.";
uint8_t Module::imagePayload[Serializer::ImageSize] = {0}; //"Lorem ipsum dolor sit amet, consectetur adipiscing elit. In efficitur urna enim, quis metus.";
sensor_msg_t Module::sensorMsg;
image_msg_t Module::imageMsg;
battery_msg_t Module::batteryMsg;
std::vector<image_msg_t> Module::broadcast_queue;
GPS Module::gps;
AHRS Module::ahrs;
MPL3115A2_Unified Module::mpl;
DHT_Unified Module::dht;
Camera Module::camera;
Serializer Module::serializer;
Logger Module::logger;

// Debugging counters
int Module::sensorCounter = 0;
int Module::imageCounter = 0;
int Module::sensorAckCounter = 0;
int Module::imageAckCounter = 0;
int Module::sensorNakCounter = 0;
int Module::imageNakCounter = 0;
