#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>

#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum {
  SEN_TYPE = 0,
  GPS_TYPE = 1,
  TPA_TYPE = 2,
  DOF_TYPE = 3,
  IMG_TYPE = 4
} msg_types;

#define BPP (1)

#define NSENSORVALUES (5)

#define I2CMESSAGESIZE (32)
#define I2CHEADERSIZE (12)
#define I2CCHUNKSIZE (I2CMESSAGESIZE - I2CHEADERSIZE)

#define MSGMESSAGESIZE (100)
#define MSGHEADERSIZE (12)
#define MSGCHUNKSIZE (MSGMESSAGESIZE - MSGHEADERSIZE)

typedef struct i2c_sensor_msg {
  uint8_t type = 0;
  float values[NSENSORVALUES] = {0.0f};
} i2c_sensor_msg_t;

typedef struct i2c_image_msg {
  uint16_t imgid = 0, chunkid = 0, nchunks = 0, w = 0, h = 0;
  uint8_t type = 0, chunksize = 0;
  uint8_t chunk[I2CCHUNKSIZE] = {0};
} i2c_image_msg_t;

typedef struct broadcast_sensor_msg {
  uint8_t type = 0, nstats = 0, status = 0, mode = 0;
  float lat = 0.0f, lon = 0.0f, alt = 0.0f, spd = 0.0f, dir = 0.0f, climb = 0.0f;
  float temp = 0.0f, baro = 0.0f, height = 0.0f;
  float humidity = 0.0f, vbattery_rpi = 0.0f, vbattery_ardu = 0.0f;
  float heading = 0.0f, pitch = 0.0f, roll = 0.0f;
} broadcast_sensor_msg_t;

typedef struct broadcast_image_msg {
  uint16_t imgid = 0, chunkid = 0, nchunks = 0, w = 0, h = 0;
  uint8_t type = 0, chunksize = 0;
  uint8_t chunk[MSGCHUNKSIZE] = {0};
} broadcast_image_msg_t;

using namespace std;

int main() {
  int w, h, bpp, chunksize = I2CCHUNKSIZE, nchunks = 0, image_offset = 0, chunk_offset = 0, chunk_id = 0, chunk_counter = 0, quality = 100;
  i2c_image_msg_t i2c_image_msg;
  i2c_sensor_msg_t i2c_sensor_msg;
  broadcast_sensor_msg_t broadcast_sensor_msg;
  broadcast_image_msg_t broadcast_image_msg;
  std::vector<i2c_image_msg_t> i2c_queue;
  std::vector<broadcast_image_msg_t> broadcast_queue;
  uint8_t *input_image = stbi_load("raspicam_greyscale_scaled.jpg", &w, &h, &bpp, BPP);

  std::cout << "Image width, height, bpp: " << w << ", " << h << ", " << bpp << std::endl;
  
  uint8_t *output_image = (uint8_t*)malloc(w*h*bpp);
  
  // Loop over some fraction of the rows of pixels
  std::memset(output_image, 0, w*h*bpp);
  for(int i = 0; i < h - 100; i++) {
    // Copy chunksize of image data to I2C message
    std::memcpy(output_image + image_offset, input_image + image_offset, w*bpp);

    // Increment image_offset
    image_offset += w*bpp;
  }

  // Save image
  stbi_write_jpg("raspicam_greyscale_scaled_incomplete.jpg", w, h, bpp, output_image, quality);
  
  // Reset output image array
  std::memset(output_image, 0, w*h*bpp);

  // Message format size
  std::cout << "Size of i2c_image_msg_t = " << sizeof(i2c_image_msg) << '\n';
  std::cout << "Size of i2c_sensor_msg_t = " << sizeof(i2c_sensor_msg) << '\n';
  std::cout << "Size of broadcast_image_msg_t = " << sizeof(broadcast_image_msg) << '\n';
  std::cout << "Size of broadcast_sensor_msg_t = " << sizeof(broadcast_sensor_msg) << '\n';
  
  // Ensure an integer number of pixels (i.e. bpp data points) fit into the message
  chunksize = (I2CCHUNKSIZE/bpp)*bpp;
  std::cout << "I2C image chunk size = " << chunksize << '\n';
  std::cout << "Broadcast image chunk size = " << MSGCHUNKSIZE << '\n';
  std::cout << "Number of I2C image chunks in broadcast image chunk = " << MSGCHUNKSIZE/chunksize << '\n';

  // Compute number of chunks
  nchunks = std::floor(static_cast<float>(w*h*bpp)/static_cast<float>(chunksize));
  
  // Add additional chunk if there is any remainder
  if(w*h*bpp % chunksize != 0) nchunks += 1;
  std::cout << "Number of I2C chunks = " << nchunks << '\n';
  std::cout << "Number of broadcast chunks = " << nchunks/(MSGCHUNKSIZE/chunksize) << '\n';
  
  // Create image chunks to send over I2C
  image_offset = 0;
  for(int i = 0; i < nchunks; i++) {
    // Copy parameters to I2C image message
    std::memcpy(i2c_image_msg.chunk, input_image + image_offset, chunksize);
    i2c_image_msg.type = IMG_TYPE;
    i2c_image_msg.imgid = 1;
    i2c_image_msg.chunkid = i + 1;
    i2c_image_msg.nchunks = nchunks;
    i2c_image_msg.chunksize = chunksize;
    i2c_image_msg.w = w;
    i2c_image_msg.h = h;
    
    // Append to I2C queue
    i2c_queue.push_back(i2c_image_msg);
  
    // Increment image_offset
    image_offset += chunksize;
  }
  
  std::cout << "I2C Queue Length = " << static_cast<int>(i2c_queue.size()) << std::endl;

  // Construct broadcast message
  chunk_offset = 0;
  chunk_id = 1;
  chunk_counter = 1;
  for(int i = 0; i < i2c_queue.size(); i++) {
    if(chunk_counter <= 4) {
      std::memcpy(broadcast_image_msg.chunk + chunk_offset, i2c_queue[i].chunk, i2c_queue[i].chunksize);
      chunk_offset += i2c_queue[i].chunksize;
      chunk_counter++;
      
      if (chunk_counter == 5) {
        chunk_counter = 1;
        chunk_offset = 0;
        
        // Copy parameters to broadcast image message
        broadcast_image_msg.type = IMG_TYPE;
        broadcast_image_msg.imgid = 1;
        broadcast_image_msg.chunkid = chunk_id++;
        broadcast_image_msg.nchunks = i2c_queue[i].nchunks/(MSGCHUNKSIZE/i2c_queue[i].chunksize);
        broadcast_image_msg.chunksize = 4*i2c_queue[i].chunksize;
        broadcast_image_msg.w = i2c_queue[i].w;
        broadcast_image_msg.h = i2c_queue[i].h;

        // Append to broadcast queue
        broadcast_queue.push_back(broadcast_image_msg);
      
        // Reset broadcast image message chunk array
        std::memset(broadcast_image_msg.chunk, 0, MSGCHUNKSIZE);
      }
    }
  }
  
  // Broadcast messages
  std::cout << "Broadcasting Queue Length = " << static_cast<int>(broadcast_queue.size()) << std::endl;
    
  // Receive messages and reconstruct image
  image_offset = 0;
  for(int i = 0; i < broadcast_queue.size(); i++) {
    //std::cout << "Broadcasting Image Chunk ID: " << static_cast<int>(broadcast_queue[i].chunkid) << std::endl;

    // Copy chunksize of image data to I2C message
    std::memcpy(output_image + image_offset, broadcast_queue[i].chunk, broadcast_queue[i].chunksize);

    // Increment image_offset
    image_offset += broadcast_queue[i].chunksize;
  }
  
  // Extract width and height of image
  w = broadcast_queue[0].w;
  h = broadcast_queue[0].h;
  
  // Save image
  stbi_write_jpg("raspicam_greyscale_scaled_reconstructed.jpg", w, h, bpp, output_image, quality);

  stbi_image_free(input_image);
  stbi_image_free(output_image);

  return 0;
}
