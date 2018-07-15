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
  enableGPS = gps.begin();

  // Initialize Orientation Sensor
  enableAHRS = ahrs.begin(i2c);
  
  // Initialize Temperature and Pressure Sensor Sensor
  enableMPL = mpl.begin(i2c);

  // Initialize Temperature and Humidity Sensor
  enableDHT = dht.begin();
  
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
  rpiBattery.value = 0.0;
  ardBattery.value = 0.0;

  while (isRunning == true) {
    currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count() >= BroadcastDelay) {
      prevTime = currentTime;
      cmdStatus = sendSPICommand(BatteryCmd, Serializer::BatterySize, response);
      if (cmdStatus == true) {
        // Store battery voltages
        rpiBattery.data[0] = response[0];
        rpiBattery.data[1] = response[1];
        rpiBattery.data[2] = response[2];
        rpiBattery.data[3] = response[3];

        ardBattery.data[0] = response[4];
        ardBattery.data[1] = response[5];
        ardBattery.data[2] = response[6];
        ardBattery.data[3] = response[7];

        // Compute message checksum, and compare with receieved checksum
        uint8_t chksum = checksum(Serializer::BatterySize - 1, response);
        if (chksum == response[Serializer::BatterySize - 1]) {
          // Print battery voltages
          if (receiveStatus == false) {
            receiveStatus = true;
            //std::cout << "RPi Battery: " << rpiBattery.value << " V, ";
            //std::cout << "Ard Battery: " << ardBattery.value << " V" << std::endl;
            std::cout << "Receieved Battery Voltages" << std::endl;
          }
        } else {
          // Received checksum does not match computed checksum
          rpiBattery.value = 0.0;
          ardBattery.value = 0.0;
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
          receiveStatus = false;
          sensorAckCounter++;
          std::cout << "Sent sensor data successfully" << std::endl;
        } else {
          // Something went wrong
          sensorNakCounter++;
          logger.error("Unable to send sensor data");
          std::cerr << "Unable to send sensor data" << std::endl;
        }

        sensorReady = false;
      } else if (imageReady == true) {
        // Else if we have image data, then send to Arduino
        cmdStatus = sendSPICommand(ImageCmd, Serializer::ImageSize, response);
        if (cmdStatus == true) {
          receiveStatus = false;
          imageAckCounter++;
          std::cout << "Sent Image Data: " << imageChunkNumber << std::endl;
        } else {
          // Something went wrong
          imageNakCounter++;
          logger.error("Unable to send image data");
          std::cerr << "Unable to send image data" << std::endl;
        }

        // Increment image chunk counter, then check if we are done
        // Otherwise load the next image chunk
        imageChunkNumber++;
        if (imageChunkNumber >= Serializer::NChunks) {
          // Reset chunk counter and set imageReady to false
          imageChunkNumber = 0;
          imageReady = false;
        } else {
          // Load next image chunk
          imageMsg.img_chunksize = Serializer::ChunkSize;
          imageMsg.img_id = imageCounter;
          imageMsg.img_chunk_id = imageChunkNumber;
          imageMsg.img_nchunks = Serializer::NChunks;
          imageMsg.img_w = Camera::WidthLowRes;
          imageMsg.img_h = Camera::HeightLowRes;
          for (uint8_t i = 0; i < imageMsg.img_chunksize; i++) {
            imageMsg.img_chunk[i] = static_cast<uint8_t>(i + 1);
          }

          // Clear imagePayload message
          for (uint8_t i = 0; i < Serializer::ImageSize; i++) {
            imagePayload[i] = 0;
          }

          // Serialize image message
          serializer.serialize(&imageMsg, imagePayload);

          // Compute and store checksum for imagePayload message
          uint8_t chksum = checksum(Serializer::ImageSize - 1, imagePayload);
          //std::cout << "Sent Checksum: 0x" << std::hex << static_cast<uint16_t>(chksum) << std::dec << std::endl;
          imagePayload[Serializer::ImageSize - 1] = chksum;
        }
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
  sensorMsg.bat_rpi = rpiBattery.value;
  sensorMsg.bat_ard = ardBattery.value;

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
      for (uint8_t i = 0; i < Serializer::SensorSize; i++) {
        sensorPayload[i] = 0;
      }

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
            // Load low resolution image from disk, and partition into NChunks

            // Populate image message
            imageMsg.img_chunksize = Serializer::ChunkSize;
            imageMsg.img_id = ++imageCounter;
            imageMsg.img_chunk_id = 0;
            imageMsg.img_nchunks = Serializer::NChunks;
            imageMsg.img_w = Camera::WidthLowRes;
            imageMsg.img_h = Camera::HeightLowRes;
            for (uint8_t i = 0; i < imageMsg.img_chunksize; i++) {
              imageMsg.img_chunk[i] = static_cast<uint8_t>(i + 1);
            }

            // Clear imagePayload message
            for (uint8_t i = 0; i < Serializer::ImageSize; i++) {
              imagePayload[i] = 0;
            }

            // Serialize image message
            serializer.serialize(&imageMsg, imagePayload);

            // Compute and store checksum for imagePayload message
            uint8_t chksum = checksum(Serializer::ImageSize - 1, imagePayload);
            //std::cout << "Sent Checksum: 0x" << std::hex << static_cast<uint16_t>(chksum) << std::dec << std::endl;
            imagePayload[Serializer::ImageSize - 1] = chksum;

            prevTime = currentTime;
            std::cout << "imageLoop: " << imageCounter << std::endl;
            imageReady = true;
          } else if (mode == Camera::VideoMode) {
            // Record video
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
int Module::imageChunkNumber = 0;
int Module::videoNumber = 1000000;
bool Module::isRunning = true;
bool Module::enableGPS = true;
bool Module::enableAHRS = true;
bool Module::enableMPL = true;
bool Module::enableDHT = true;
bool Module::enableIMG = true;
bool Module::recordVideo = false;
uint8_t Module::sensorPayload[Serializer::SensorSize] = {0}; //"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut eu volutpat.";
uint8_t Module::imagePayload[Serializer::ImageSize] = {0}; //"Lorem ipsum dolor sit amet, consectetur adipiscing elit. In efficitur urna enim, quis metus.";
sensor_msg_t Module::sensorMsg;
image_msg_t Module::imageMsg;
floatunion_t Module::rpiBattery;
floatunion_t Module::ardBattery;
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
