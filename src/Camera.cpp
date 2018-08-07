#include "HABPi.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Camera Constructor
Camera::Camera() {
  // Load VGA palette RGB values
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0xa8});
  palette.push_back({0x0,0xa8,0x0});
  palette.push_back({0x0,0xa8,0xa8});
  palette.push_back({0xa8,0x0,0x0});
  palette.push_back({0xa8,0x0,0xa8});
  palette.push_back({0xa8,0x57,0x0});
  palette.push_back({0xa8,0xa8,0xa8});
  palette.push_back({0x57,0x57,0x57});
  palette.push_back({0x57,0x57,0xff});
  palette.push_back({0x57,0xff,0x57});
  palette.push_back({0x57,0xff,0xff});
  palette.push_back({0xff,0x57,0x57});
  palette.push_back({0xff,0x57,0xff});
  palette.push_back({0xff,0xff,0x57});
  palette.push_back({0xff,0xff,0xff});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x17,0x17,0x17});
  palette.push_back({0x20,0x20,0x20});
  palette.push_back({0x2f,0x2f,0x2f});
  palette.push_back({0x38,0x38,0x38});
  palette.push_back({0x47,0x47,0x47});
  palette.push_back({0x50,0x50,0x50});
  palette.push_back({0x60,0x60,0x60});
  palette.push_back({0x70,0x70,0x70});
  palette.push_back({0x80,0x80,0x80});
  palette.push_back({0x90,0x90,0x90});
  palette.push_back({0xa0,0xa0,0xa0});
  palette.push_back({0xb7,0xb7,0xb7});
  palette.push_back({0xc8,0xc8,0xc8});
  palette.push_back({0xe0,0xe0,0xe0});
  palette.push_back({0xff,0xff,0xff});
  palette.push_back({0x0,0x0,0xff});
  palette.push_back({0x40,0x0,0xff});
  palette.push_back({0x7f,0x0,0xff});
  palette.push_back({0xbf,0x0,0xff});
  palette.push_back({0xff,0x0,0xff});
  palette.push_back({0xff,0x0,0xbf});
  palette.push_back({0xff,0x0,0x7f});
  palette.push_back({0xff,0x0,0x40});
  palette.push_back({0xff,0x0,0x0});
  palette.push_back({0xff,0x40,0x0});
  palette.push_back({0xff,0x7f,0x0});
  palette.push_back({0xff,0xbf,0x0});
  palette.push_back({0xff,0xff,0x0});
  palette.push_back({0xbf,0xff,0x0});
  palette.push_back({0x7f,0xff,0x0});
  palette.push_back({0x40,0xff,0x0});
  palette.push_back({0x0,0xff,0x0});
  palette.push_back({0x0,0xff,0x40});
  palette.push_back({0x0,0xff,0x7f});
  palette.push_back({0x0,0xff,0xbf});
  palette.push_back({0x0,0xff,0xff});
  palette.push_back({0x0,0xbf,0xff});
  palette.push_back({0x0,0x7f,0xff});
  palette.push_back({0x0,0x40,0xff});
  palette.push_back({0x7f,0x7f,0xff});
  palette.push_back({0x9f,0x7f,0xff});
  palette.push_back({0xbf,0x7f,0xff});
  palette.push_back({0xdf,0x7f,0xff});
  palette.push_back({0xff,0x7f,0xff});
  palette.push_back({0xff,0x7f,0xdf});
  palette.push_back({0xff,0x7f,0xbf});
  palette.push_back({0xff,0x7f,0x9f});
  palette.push_back({0xff,0x7f,0x7f});
  palette.push_back({0xff,0x9f,0x7f});
  palette.push_back({0xff,0xbf,0x7f});
  palette.push_back({0xff,0xdf,0x7f});
  palette.push_back({0xff,0xff,0x7f});
  palette.push_back({0xdf,0xff,0x7f});
  palette.push_back({0xbf,0xff,0x7f});
  palette.push_back({0x9f,0xff,0x7f});
  palette.push_back({0x7f,0xff,0x7f});
  palette.push_back({0x7f,0xff,0x9f});
  palette.push_back({0x7f,0xff,0xbf});
  palette.push_back({0x7f,0xff,0xdf});
  palette.push_back({0x7f,0xff,0xff});
  palette.push_back({0x7f,0xdf,0xff});
  palette.push_back({0x7f,0xbf,0xff});
  palette.push_back({0x7f,0x9f,0xff});
  palette.push_back({0xb7,0xb7,0xff});
  palette.push_back({0xc7,0xb7,0xff});
  palette.push_back({0xd8,0xb7,0xff});
  palette.push_back({0xe8,0xb7,0xff});
  palette.push_back({0xff,0xb7,0xff});
  palette.push_back({0xff,0xb7,0xe8});
  palette.push_back({0xff,0xb7,0xd8});
  palette.push_back({0xff,0xb7,0xc7});
  palette.push_back({0xff,0xb7,0xb7});
  palette.push_back({0xff,0xc7,0xb7});
  palette.push_back({0xff,0xd8,0xb7});
  palette.push_back({0xff,0xe8,0xb7});
  palette.push_back({0xff,0xff,0xb7});
  palette.push_back({0xe8,0xff,0xb7});
  palette.push_back({0xd8,0xff,0xb7});
  palette.push_back({0xc7,0xff,0xb7});
  palette.push_back({0xb7,0xff,0xb7});
  palette.push_back({0xb7,0xff,0xc7});
  palette.push_back({0xb7,0xff,0xd8});
  palette.push_back({0xb7,0xff,0xe8});
  palette.push_back({0xb7,0xff,0xff});
  palette.push_back({0xb7,0xe8,0xff});
  palette.push_back({0xb7,0xd8,0xff});
  palette.push_back({0xb7,0xc7,0xff});
  palette.push_back({0x0,0x0,0x70});
  palette.push_back({0x1f,0x0,0x70});
  palette.push_back({0x38,0x0,0x70});
  palette.push_back({0x57,0x0,0x70});
  palette.push_back({0x70,0x0,0x70});
  palette.push_back({0x70,0x0,0x57});
  palette.push_back({0x70,0x0,0x38});
  palette.push_back({0x70,0x0,0x1f});
  palette.push_back({0x70,0x0,0x0});
  palette.push_back({0x70,0x1f,0x0});
  palette.push_back({0x70,0x38,0x0});
  palette.push_back({0x70,0x57,0x0});
  palette.push_back({0x70,0x70,0x0});
  palette.push_back({0x57,0x70,0x0});
  palette.push_back({0x38,0x70,0x0});
  palette.push_back({0x1f,0x70,0x0});
  palette.push_back({0x0,0x70,0x0});
  palette.push_back({0x0,0x70,0x1f});
  palette.push_back({0x0,0x70,0x38});
  palette.push_back({0x0,0x70,0x57});
  palette.push_back({0x0,0x70,0x70});
  palette.push_back({0x0,0x57,0x70});
  palette.push_back({0x0,0x38,0x70});
  palette.push_back({0x0,0x1f,0x70});
  palette.push_back({0x38,0x38,0x70});
  palette.push_back({0x47,0x38,0x70});
  palette.push_back({0x57,0x38,0x70});
  palette.push_back({0x60,0x38,0x70});
  palette.push_back({0x70,0x38,0x70});
  palette.push_back({0x70,0x38,0x60});
  palette.push_back({0x70,0x38,0x57});
  palette.push_back({0x70,0x38,0x47});
  palette.push_back({0x70,0x38,0x38});
  palette.push_back({0x70,0x47,0x38});
  palette.push_back({0x70,0x57,0x38});
  palette.push_back({0x70,0x60,0x38});
  palette.push_back({0x70,0x70,0x38});
  palette.push_back({0x60,0x70,0x38});
  palette.push_back({0x57,0x70,0x38});
  palette.push_back({0x47,0x70,0x38});
  palette.push_back({0x38,0x70,0x38});
  palette.push_back({0x38,0x70,0x47});
  palette.push_back({0x38,0x70,0x57});
  palette.push_back({0x38,0x70,0x60});
  palette.push_back({0x38,0x70,0x70});
  palette.push_back({0x38,0x60,0x70});
  palette.push_back({0x38,0x57,0x70});
  palette.push_back({0x38,0x47,0x70});
  palette.push_back({0x50,0x50,0x70});
  palette.push_back({0x58,0x50,0x70});
  palette.push_back({0x60,0x50,0x70});
  palette.push_back({0x68,0x50,0x70});
  palette.push_back({0x70,0x50,0x70});
  palette.push_back({0x70,0x50,0x68});
  palette.push_back({0x70,0x50,0x60});
  palette.push_back({0x70,0x50,0x58});
  palette.push_back({0x70,0x50,0x50});
  palette.push_back({0x70,0x58,0x50});
  palette.push_back({0x70,0x60,0x50});
  palette.push_back({0x70,0x68,0x50});
  palette.push_back({0x70,0x70,0x50});
  palette.push_back({0x68,0x70,0x50});
  palette.push_back({0x60,0x70,0x50});
  palette.push_back({0x58,0x70,0x50});
  palette.push_back({0x50,0x70,0x50});
  palette.push_back({0x50,0x70,0x58});
  palette.push_back({0x50,0x70,0x60});
  palette.push_back({0x50,0x70,0x68});
  palette.push_back({0x50,0x70,0x70});
  palette.push_back({0x50,0x68,0x70});
  palette.push_back({0x50,0x60,0x70});
  palette.push_back({0x50,0x58,0x70});
  palette.push_back({0x0,0x0,0x40});
  palette.push_back({0x10,0x0,0x40});
  palette.push_back({0x20,0x0,0x40});
  palette.push_back({0x30,0x0,0x40});
  palette.push_back({0x40,0x0,0x40});
  palette.push_back({0x40,0x0,0x30});
  palette.push_back({0x40,0x0,0x20});
  palette.push_back({0x40,0x0,0x10});
  palette.push_back({0x40,0x0,0x0});
  palette.push_back({0x40,0x10,0x0});
  palette.push_back({0x40,0x20,0x0});
  palette.push_back({0x40,0x30,0x0});
  palette.push_back({0x40,0x40,0x0});
  palette.push_back({0x30,0x40,0x0});
  palette.push_back({0x20,0x40,0x0});
  palette.push_back({0x10,0x40,0x0});
  palette.push_back({0x0,0x40,0x0});
  palette.push_back({0x0,0x40,0x10});
  palette.push_back({0x0,0x40,0x20});
  palette.push_back({0x0,0x40,0x30});
  palette.push_back({0x0,0x40,0x40});
  palette.push_back({0x0,0x30,0x40});
  palette.push_back({0x0,0x20,0x40});
  palette.push_back({0x0,0x10,0x40});
  palette.push_back({0x20,0x20,0x40});
  palette.push_back({0x28,0x20,0x40});
  palette.push_back({0x30,0x20,0x40});
  palette.push_back({0x38,0x20,0x40});
  palette.push_back({0x40,0x20,0x40});
  palette.push_back({0x40,0x20,0x38});
  palette.push_back({0x40,0x20,0x30});
  palette.push_back({0x40,0x20,0x28});
  palette.push_back({0x40,0x20,0x20});
  palette.push_back({0x40,0x28,0x20});
  palette.push_back({0x40,0x30,0x20});
  palette.push_back({0x40,0x38,0x20});
  palette.push_back({0x40,0x40,0x20});
  palette.push_back({0x38,0x40,0x20});
  palette.push_back({0x30,0x40,0x20});
  palette.push_back({0x28,0x40,0x20});
  palette.push_back({0x20,0x40,0x20});
  palette.push_back({0x20,0x40,0x28});
  palette.push_back({0x20,0x40,0x30});
  palette.push_back({0x20,0x40,0x38});
  palette.push_back({0x20,0x40,0x40});
  palette.push_back({0x20,0x38,0x40});
  palette.push_back({0x20,0x30,0x40});
  palette.push_back({0x20,0x28,0x40});
  palette.push_back({0x2f,0x2f,0x40});
  palette.push_back({0x30,0x2f,0x40});
  palette.push_back({0x37,0x2f,0x40});
  palette.push_back({0x3f,0x2f,0x40});
  palette.push_back({0x40,0x2f,0x40});
  palette.push_back({0x40,0x2f,0x3f});
  palette.push_back({0x40,0x2f,0x37});
  palette.push_back({0x40,0x2f,0x30});
  palette.push_back({0x40,0x2f,0x2f});
  palette.push_back({0x40,0x30,0x2f});
  palette.push_back({0x40,0x37,0x2f});
  palette.push_back({0x40,0x3f,0x2f});
  palette.push_back({0x40,0x40,0x2f});
  palette.push_back({0x3f,0x40,0x2f});
  palette.push_back({0x37,0x40,0x2f});
  palette.push_back({0x30,0x40,0x2f});
  palette.push_back({0x2f,0x40,0x2f});
  palette.push_back({0x2f,0x40,0x30});
  palette.push_back({0x2f,0x40,0x37});
  palette.push_back({0x2f,0x40,0x3f});
  palette.push_back({0x2f,0x40,0x40});
  palette.push_back({0x2f,0x3f,0x40});
  palette.push_back({0x2f,0x37,0x40});
  palette.push_back({0x2f,0x30,0x40});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
  palette.push_back({0x0,0x0,0x0});
}

// Camera Destructor
Camera::~Camera() {}

// Begin Camera
bool Camera::begin() {
	std::string delimiter = "detected=";
	std::string result = Global::exec("vcgencmd get_camera");

	size_t pos = 0;
	std::string token;
	while ((pos = result.find(delimiter)) != std::string::npos) {
		token = result.substr(0, pos);
		result.erase(0, pos + delimiter.length());
	}
  if(result == "0") {
    Module::logger.error("Camera is not connected");
    return true;
  } else {
    Module::logger.info("Camera is connected");
    return true;
  }
}

// Update Camera
void Camera::update(uint8_t mode) {
	switch(mode) {
		case ImageMode: {
			capture();
			break;
		}
		case VideoMode: {
			record();
			break;
		}
		default: {
			std::ostringstream msg;
			msg << "Unrecognized Camera Mode: 0x" << std::hex << static_cast<uint16_t>(mode);
			std::string msgStr = msg.str();
			Module::logger.error(msgStr.c_str());
			break;
		}
	}
}

// Capture Image
void Camera::capture() {
	// Construct command to capture large size image
  std::ostringstream cmdLarge;
  cmdLarge << "raspistill --nopreview --thumb none";
  cmdLarge << " --sharpness " << Sharpness;
  cmdLarge << " --exposure " << Exposure;
  cmdLarge << " --rotation " << Rotation;
  cmdLarge << " --width " << WidthLarge;
  cmdLarge << " --height " << HeightLarge;
  cmdLarge << " --quality " << Quality;
  cmdLarge << " --timeout " << Timeout;
  cmdLarge << " --encoding " << ImageEncoding;
  cmdLarge << " --output images/image_" << Module::imageNumber << "." << ImageEncoding;
  std::string cmdLargeStr = cmdLarge.str();

  // Execute large size image command
  system(cmdLargeStr.c_str());

  // Construct command to capture small size image
  std::ostringstream cmdSmall;
  cmdSmall << "raspistill --nopreview --thumb none";
  cmdSmall << " --sharpness " << Sharpness;
  cmdSmall << " --exposure " << Exposure;
  cmdSmall << " --rotation " << Rotation;
  cmdSmall << " --width " << WidthSmall;
  cmdSmall << " --height " << HeightSmall;
  cmdSmall << " --quality " << Quality;
  cmdSmall << " --timeout " << Timeout;
  cmdSmall << " --encoding " << ImageEncoding;
  cmdSmall << " --output images/thumbnail." << ImageEncoding;
  std::string cmdSmallStr = cmdSmall.str();

  // Execute small size image command
  system(cmdSmallStr.c_str());

  // Construct command to dither and down-sample with VGA palatte
  // e.g. convert thumbnail.png -alpha off -colors 256 +dither -remap config/vga.png thumbnail_vga.png
  std::ostringstream cmdVga;
  cmdVga << "convert images/thumbnail." << ImageEncoding;
  cmdVga << " -alpha off";
  cmdVga << " -colors " << NColours;
  cmdVga << " +dither";
  cmdVga << " -remap " << VgaPalette;
  cmdVga << " images/thumbnail_vga." << ImageEncoding;
  std::string cmdVgaStr = cmdVga.str();

  // Execute VGA dither command
  system(cmdVgaStr.c_str());
}

// Record Video
void Camera::record() {
	// Construct command to record a video
  std::ostringstream cmd;
  cmd << "raspivid --nopreview --vstab";
  cmd << " --sharpness " << Sharpness;
  cmd << " --exposure " << Exposure;
  cmd << " --rotation " << Rotation;
  cmd << " --timeout " << Timeout * 8;
  cmd << " --output images/video_" << Module::videoNumber << "." << VideoEncoding;
  Module::videoNumber++;
  std::string cmdStr = cmd.str();

  // Execute record command
  system(cmdStr.c_str());
}

// Load and Partition Image Data
void Camera::load() {
  int w, h, bpp, image_offset = 0, stride = 0;
  image_msg_t imageMsg;
  uint8_t *input_image, *image_broadcast;

  // Load the thumbnail image
  input_image = stbi_load("images/thumbnail_vga.png", &w, &h, &bpp, Bpp); 

  //std::cout << "Image width, height, bpp: " << w << ", " << h << ", " << bpp << std::endl;

  // Define size of broadcast image
  image_broadcast = new uint8_t[w*h];

  // Replace three RGB bytes with VGA palette index byte
  std::memset(image_broadcast, 0, w*h);
  for(int i = 0; i < h; i++) {
    for(int j = 0; j < w; j++) {
      rgb_t p;
      p.r = input_image[i*w*bpp + j*bpp + 0];
      p.g = input_image[i*w*bpp + j*bpp + 1];
      p.b = input_image[i*w*bpp + j*bpp + 2];

      bool found = false;
      for(int k = 0; k < static_cast<int>(palette.size()); k++) {
        if(palette[k].compare(p) == true) {
          found = true;
          image_broadcast[image_offset++] = static_cast<uint8_t>(k);
        }
      }
      if(found == false) {
        // Set unknown colours to (r, g, b) = (0xFF, 0x0, 0xFF) @ index 36
        image_broadcast[image_offset++] = static_cast<uint8_t>(36);
      }
    }
  }

  // Save broadcast image
  stbi_write_png("images/thumbnail_broadcast.png", w, h, 1, image_broadcast, stride);

  // Compute number of chunks
  Serializer::NChunks = std::floor(static_cast<float>(w*h)/static_cast<float>(Serializer::ChunkSize));
  
  // Add additional chunk if there is any remainder
  if(w*h % Serializer::ChunkSize != 0) Serializer::NChunks += 1;
  //std::cout << "Number of image chunks = " << Serializer::NChunks << '\n';

  // Clear broadcast queue
  Module::broadcast_queue.clear();

  // Create image messages to broadcast
  image_offset = 0;
  for(int i = 0; i < Serializer::NChunks; i++) {
    // Copy parameters to image message
    for(int j = 0; j < Serializer::ChunkSize; j++) {
      imageMsg.img_chunk[j] = image_broadcast[image_offset + j];
    }
    
    imageMsg.type = Module::ImageCmd;
    imageMsg.img_id = Module::imageNumber;
    imageMsg.img_chunk_id = i;
    imageMsg.img_nchunks = Serializer::NChunks;
    imageMsg.img_chunksize = Serializer::ChunkSize;
    imageMsg.img_w = w;
    imageMsg.img_h = h;
    
    // Append to broadcast queue
    Module::broadcast_queue.push_back(imageMsg);

    // Increment image_offset
    image_offset += Serializer::ChunkSize;
  }

  // Increment image number
  Module::imageNumber++;

  // Free resources
  stbi_image_free(input_image);
  delete [] image_broadcast;
}

const std::string Camera::ImageEncoding = "png";
const std::string Camera::VideoEncoding = "h264";
const std::string Camera::VgaPalette = "config/vga.png";
const std::string Camera::Exposure = "auto";