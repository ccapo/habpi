#include "HABPi.h"

i2c_bus::i2c_bus(): fd(-1) {}

i2c_bus::i2c_bus(const std::string &name): fd(-1) {
  open(name);
  Module::logger.info("Connected to I2C bus");
}

i2c_bus::i2c_bus(const i2c_bus &other): fd(-1) {
  *this = other;
  Module::logger.info("Connected to I2C bus");
}

i2c_bus & i2c_bus::operator=(const i2c_bus &other) {
  if (other.fd == -1) {
    close();
  } else {
    open_from_fd(other.fd);
  }
  return *this;
}

i2c_bus::~i2c_bus() {}

void i2c_bus::open(const std::string &name) {
  close();
  fd = ::open(name.c_str(), O_RDWR);
  if (fd == -1) {
    std::string msg;
    msg = std::string("Failed to open I2C device ") + name;
    Module::logger.error(msg.c_str());
  } else {
    Module::logger.info("Connected to I2C bus");
  }
}

void i2c_bus::open_from_fd(int other_fd) {
  close();
  fd = dup(other_fd);
  if (fd == -1) {
    Module::logger.error("Failed to dup I2C device");
  } else {
    Module::logger.info("Connected to I2C bus");
  }
}

void i2c_bus::close() {
  if (fd != -1) {
    ::close(fd);
    fd = -1;
    Module::logger.info("Disconnected from I2C bus");
  }
}

void i2c_bus::write_byte_and_read(uint8_t address, uint8_t command, uint8_t *data, size_t size) {
  i2c_msg messages[2] = {
    { address, 0, 1, (typeof(i2c_msg().buf)) &command },
    { address, I2C_M_RD, (typeof(i2c_msg().len)) size, (typeof(i2c_msg().buf)) data },
  };
  i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };

  int result = ioctl(fd, I2C_RDWR, &ioctl_data);

  if (result != 2) {
    if (Global::Debug) Module::logger.error("Failed to read to I2C");
  }
}

void i2c_bus::write(uint8_t address, uint8_t *data, size_t size) {
  i2c_msg messages[1] = {
    { address, 0, (typeof(i2c_msg().len)) size, (typeof(i2c_msg().buf)) data }
  };
  i2c_rdwr_ioctl_data ioctl_data = { messages, 1 };

  int result = ioctl(fd, I2C_RDWR, &ioctl_data);

  if (result != 1) {
    if (Global::Debug) Module::logger.error("Failed to write to I2C");
  }
}

int i2c_bus::try_write_byte_and_read(uint8_t address, uint8_t byte, uint8_t *data, size_t size) {
  i2c_msg messages[2] = {
    { address, 0, 1, (typeof(i2c_msg().buf)) &byte },
    { address, I2C_M_RD, (typeof(i2c_msg().len))size, (typeof(i2c_msg().buf))data },
  };
  i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };

  int result = ioctl(fd, I2C_RDWR, &ioctl_data);

  if (result != 2) {
    return -1;
  }

  return 0;
}
