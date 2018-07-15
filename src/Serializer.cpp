#include "HABPi.h"

// Serializer Constructor
Serializer::Serializer() {}

// Serializer Destructor
Serializer::~Serializer() {}

/* Serialize Sensor Message */
void Serializer::serialize(sensor_msg_t *msg, uint8_t *data) {
  // Message Type
  uint8_t *p = (uint8_t *)data;
  *p = msg->type;
  p++;

  // Proximity flag
  *p = msg->proximityFlag;
  p++;

  // GPS
  *p = msg->gps_nsats;
  p++;
  *p = msg->gps_status;
  p++;
  *p = msg->gps_mode;
  p++;

  float *q = (float *)p;
  *q = msg->gps_lat;
  q++;
  *q = msg->gps_lon;
  q++;
  *q = msg->gps_alt;
  q++;
  *q = msg->gps_spd;
  q++;
  *q = msg->gps_dir;
  q++;
  *q = msg->gps_cli;
  q++;

  // AHRS
  *q = msg->ahrs_head;
  q++;
  *q = msg->ahrs_pitch;
  q++;
  *q = msg->ahrs_roll;
  q++;

  // MPL
  *q = msg->mpl_temp;
  q++;
  *q = msg->mpl_pres;
  q++;
  *q = msg->mpl_alt;
  q++;

  // DHT + VBAT
  *q = msg->dht_temp;
  q++;
  *q = msg->dht_relh;
  q++;
  *q = msg->bat_rpi;
  q++;
  *q = msg->bat_ard;
  q++;
}

// Serialize Image Message
void Serializer::serialize(image_msg_t *msg, uint8_t *data) {
  uint8_t *p = (uint8_t *)data;
  *p = msg->type;
  p++;
  *p = msg->img_chunksize;
  p++;

  uint16_t *q = (uint16_t *)p;
  *q = msg->img_id;
  q++;    
  *q = msg->img_chunk_id;
  q++;    
  *q = msg->img_nchunks;
  q++;
  *q = msg->img_w;
  q++;
  *q = msg->img_h;
  q++;

  uint8_t *r = (uint8_t *)q;
  for (uint16_t i = 0; i < ChunkSize; i++) {
    *r = msg->img_chunk[i];
    r++;
  }
}

// Deserialize Sensor Message
void Serializer::deserialize(uint8_t *data, sensor_msg_t *msg) {
  // Message Type
  uint8_t *p = (uint8_t *)data;
  msg->type = *p;
  p++;

  // Proximity Flag
  msg->proximityFlag = *p;
  p++;

  // GPS
  msg->gps_nsats = *p;
  p++;
  msg->gps_status = *p;
  p++;
  msg->gps_mode = *p;
  p++;

  float *q = (float *)p;
  msg->gps_lat = *q;
  q++;
  msg->gps_lon = *q;
  q++;
  msg->gps_alt = *q;
  q++;
  msg->gps_spd = *q;
  q++;
  msg->gps_dir = *q;
  q++;
  msg->gps_cli = *q;
  q++;

  // AHRS
  msg->ahrs_head = *q;
  q++;
  msg->ahrs_pitch = *q;
  q++;
  msg->ahrs_roll = *q;
  q++;

  // MPL
  msg->mpl_temp = *q;
  q++;
  msg->mpl_pres = *q;
  q++;
  msg->mpl_alt = *q;
  q++;

  // DHT + VBAT
  msg->dht_temp = *q;
  q++;
  msg->dht_relh = *q;
  q++;
  msg->bat_rpi = *q;
  q++;
  msg->bat_ard = *q;
  q++;
}

// Deserialize Image Message
void Serializer::deserialize(uint8_t *data, image_msg_t *msg) {
  // Message Type
  uint8_t *p = (uint8_t *)data;
  msg->type = *p;
  p++;
  msg->img_chunksize = *p;
  p++;

  uint16_t *q = (uint16_t *)p;
  msg->img_id = *q;
  q++;    
  msg->img_chunk_id = *q;
  q++;    
  msg->img_nchunks = *q;
  q++;
  msg->img_w = *q;
  q++;
  msg->img_h = *q;
  q++;

  uint8_t *r = (uint8_t *)q;
  for (uint16_t i = 0; i < ChunkSize; i++) {
    msg->img_chunk[i] = *r;
    r++;
  }
}

// Print Sensor Message
void Serializer::print(sensor_msg_t msg) {
  std::cout << "Type        = 0x" << std::hex << static_cast<uint16_t>(msg.type) << std::dec << std::endl;

  // GPS
  std::cout << "GPS" << std::endl;
  std::cout << "nsats       = " << static_cast<uint16_t>(msg.gps_nsats) << std::endl;
  std::cout << "status      = " << static_cast<uint16_t>(msg.gps_status) << std::endl;
  std::cout << "mode        = " << static_cast<uint16_t>(msg.gps_mode) << std::endl;
  std::cout << "lat         = " << msg.gps_lat << std::endl;
  std::cout << "lon         = " << msg.gps_lon << std::endl;
  std::cout << "alt         = " << msg.gps_alt << std::endl;
  std::cout << "dir         = " << msg.gps_dir << std::endl;
  std::cout << "spd         = " << msg.gps_spd << std::endl;
  std::cout << "cli         = " << msg.gps_cli << std::endl;

  // AHRS
  std::cout << "AHRS" << std::endl;
  std::cout << "heading     = " << msg.ahrs_head << std::endl;
  std::cout << "pitch       = " << msg.ahrs_pitch << std::endl;
  std::cout << "roll        = " << msg.ahrs_roll << std::endl;

  // MPL
  std::cout << "MPL" << std::endl;
  std::cout << "temperature = " << msg.mpl_temp << std::endl;
  std::cout << "pressure    = " << msg.mpl_pres << std::endl;
  std::cout << "height      = " << msg.mpl_alt << std::endl;

  // DHT + VBAT
  std::cout << "DHT + VBAT" << std::endl;
  std::cout << "temperature = " << msg.dht_temp << std::endl;
  std::cout << "humidity    = " << msg.dht_relh << std::endl;
  std::cout << "bat_rpi     = " << msg.bat_rpi << std::endl;
  std::cout << "bat_ard     = " << msg.bat_ard << std::endl;

  std::cout << std::endl;
}

// Print Image Message
void Serializer::print(image_msg_t msg) {
  std::cout << "Type        = 0x" << std::hex << static_cast<uint16_t>(msg.type) << std::dec << std::endl;

  // IMG
  std::cout << "IMG" << std::endl;
  std::cout << "chunksize   = " << static_cast<uint16_t>(msg.img_chunksize) << std::endl;
  std::cout << "img_id      = " << msg.img_id << std::endl;
  std::cout << "chunk_id    = " << msg.img_chunk_id << std::endl;
  std::cout << "nchunks     = " << msg.img_nchunks << std::endl;
  std::cout << "w           = " << msg.img_w << std::endl;
  std::cout << "h           = " << msg.img_h << std::endl;
  std::cout << "chunk       = " << msg.img_chunk << std::endl;

  std::cout << std::endl;
}
