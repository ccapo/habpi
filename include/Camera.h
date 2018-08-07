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

#include "Serializer.h"

// RGB values
struct rgb_t {
  uint8_t r, g, b;

  // RGB comparison method
  bool compare(rgb_t p) {
    bool status = true;
    status = status && this->r == p.r;
    status = status && this->g == p.g;
    status = status && this->b == p.b;
    return status;
  }
};

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

  // Load and Partition Image Data
  void load();

  // VGA palette RGB values
  std::vector<rgb_t> palette;

  // Static Constants

  // Height and Width for Large Images
  static const int HeightLarge = 2464;
  static const int WidthLarge = 3280;

  // Height and Width for Small Images
  static const int HeightSmall = 240;
  static const int WidthSmall = 320;

  static const bool Preview = false;
  static const bool Vstab = true;
  static const int Timeout = 2000;
  static const int Rotation = 180;
  static const int Quality = 100;
  static const int Sharpness = 100;
  static const int Bpp = 0;
  static const int NColours = 255;

  static const std::string ImageEncoding;
  static const std::string VideoEncoding;
  static const std::string VgaPalette;
  static const std::string Exposure;

  // Camera Mode
  static const uint8_t ImageMode = 0x80;
  static const uint8_t VideoMode = 0xFF;
};