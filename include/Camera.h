/**
 * Raspberry Pi Camera
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

class Camera {
public:
  // Camera Constructor
  Camera();

  // Camera Destructor
  ~Camera();

  // Begin Camera
  bool begin();

  // Update Camera
  void update(uint8_t mode);

  // Capture Image
  void capture();

  // Record Video
  void record();

  // Static Constants

  // Height and Width for High Resolution Images
  static const int HeightHiRes = 2464;
  static const int WidthHiRes = 3280;

  // Height and Width for Low Resolution Images
  static const int HeightLowRes = 240;
  static const int WidthLowRes = 320;

  static const bool Preview = false;
  static const bool Vstab = true;
  static const int Timeout = 2000;
  static const int Rotation = 180;
  static const int Quality = 100;
  static const int Sharpness = 100;

  static const std::string Encoding;
  static const std::string Exposure;

  // Camera Mode
  static const uint8_t ImageMode = 0x80;
  static const uint8_t VideoMode = 0xFF;
};