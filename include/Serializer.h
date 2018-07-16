#pragma once

#include <iostream>
#include <cstring>

// Broadcast message parameters
#define PAYLOADSIZE          (100)
#define HEADERSIZE           (12)
#define CHUNKSIZE            (80)

// Sensor message
struct sensor_msg_t {
  // Message Type and Proximity Flag
  uint8_t type, proximityFlag;
  
  // GPS Data
  uint8_t gps_nsats, gps_status, gps_mode;
  float gps_lat, gps_lon, gps_alt, gps_spd, gps_dir, gps_cli;
  
  // MPL3115A2 Data
  float mpl_temp, mpl_pres, mpl_alt;

  // AHRS: NXP_FXOS8700_FXAS21002C Data
  float ahrs_head, ahrs_pitch, ahrs_roll;

  // DHT11 Data
  float dht_temp, dht_relh;
  
  // Battery Data
  float bat_rpi, bat_ard;
};

// Image message
struct image_msg_t {
  // Message Type
  uint8_t type;
  
  // Image Data
  uint8_t img_chunksize, img_chunk[CHUNKSIZE];
  uint16_t img_id, img_chunk_id, img_nchunks, img_w, img_h;
};

// Battery message
struct battery_msg_t {
  // Message Type
  uint8_t type;
  
  // Battery Data
  float bat_rpi, bat_ard;
};

class Serializer {
public:
  // Constructor
  Serializer();
  
  // Destructor
  ~Serializer();

  // Serialize Sensor Message
  void serialize(sensor_msg_t *msg, uint8_t *data);

  // Serialize Image Message
  void serialize(image_msg_t *msg, uint8_t *data);
  
  // Deserialize Sensor Message
  void deserialize(uint8_t *data, sensor_msg_t *msg);

  // Deserialize Image Message
  void deserialize(uint8_t *data, image_msg_t *msg);
  
  // Print Sensor Message
  void print(sensor_msg_t msg);

  // Print Image Message
  void print(image_msg_t msg);

  // Static Constants
  static const int PayloadSize = PAYLOADSIZE;             // Including checksum
  static const int BatterySize = 9;                       // Including checksum
  static const int ImageSize = 1 + sizeof(image_msg_t);   // Including checksum
  static const int SensorSize = 1 + sizeof(sensor_msg_t); // Including checksum
  static const int NChunks = 960;
  static const int ChunkSize = CHUNKSIZE;
};
