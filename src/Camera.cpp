#include "HABPi.h"

// Camera Constructor
Camera::Camera() {}

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
	// Construct raspistill command for high resolution image
  std::ostringstream cmdHigh;
  cmdHigh << "raspistill --nopreview --thumb none";
  cmdHigh << " --sharpness " << Sharpness;
  cmdHigh << " --exposure " << Exposure;
  cmdHigh << " --rotation " << Rotation;
  cmdHigh << " --width " << WidthHiRes;
  cmdHigh << " --height " << HeightHiRes;
  cmdHigh << " --quality " << Quality;
  cmdHigh << " --timeout " << Timeout;
  cmdHigh << " --encoding " << Encoding;
  cmdHigh << " --output images/image_" << Module::imageNumber << "." << Encoding;
  Module::imageNumber++;
  std::string cmdHighStr = cmdHigh.str();

  // Execute high resolution image command
  system(cmdHighStr.c_str());

  // Construct raspistill command for low resolution image
  std::ostringstream cmdLow;
  cmdLow << "raspistill --nopreview --thumb none";
  cmdLow << " --sharpness " << Sharpness;
  cmdLow << " --exposure " << Exposure;
  cmdLow << " --rotation " << Rotation;
  cmdLow << " --width " << WidthLowRes;
  cmdLow << " --height " << HeightLowRes;
  cmdLow << " --quality " << Quality;
  cmdLow << " --timeout " << Timeout;
  cmdLow << " --encoding " << Encoding;
  cmdLow << " --output images/thumbnail." << Encoding;
  std::string cmdLowStr = cmdLow.str();

  // Execute low resolution image command
  system(cmdLowStr.c_str());
}

// Record Video
void Camera::record() {
	// Construct raspivid command
  std::ostringstream cmd;
  cmd << "raspivid --nopreview --vstab";
  cmd << " --sharpness " << Sharpness;
  cmd << " --exposure " << Exposure;
  cmd << " --rotation " << Rotation;
  cmd << " --timeout " << Timeout * 8;
  cmd << " --output images/video_" << Module::videoNumber << ".h264";
  Module::videoNumber++;
  std::string cmdStr = cmd.str();

  // Execute command
  system(cmdStr.c_str());
}

const std::string Camera::Encoding = "jpg";
const std::string Camera::Exposure = "auto";