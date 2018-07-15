#include "HABPi.h"

spi_bus::spi_bus(): fd(-1), speed(SpiSpeed) {}

spi_bus::spi_bus(const std::string &name): fd(-1), speed(SpiSpeed) {
  fd = ::open(name.c_str(), O_RDWR);
  ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  Module::logger.info("Connected to SPI bus");
}

spi_bus::spi_bus(const spi_bus &other): fd(-1), speed(SpiSpeed) {
  *this = other;
  Module::logger.info("Connected to SPI bus");
}

spi_bus & spi_bus::operator=(const spi_bus &other) {
  if (other.fd == -1) {
    close();
  } else {
    open_from_fd(other.fd);
  }
  return *this;
}

spi_bus::~spi_bus() {}

void spi_bus::open(const std::string &name) {
  close();
  fd = ::open(name.c_str(), O_RDWR);
  if (fd == -1) {
    std::string msg;
    msg = std::string("Failed to open SPI device ") + name;
    Module::logger.error(msg.c_str());
  }
  Module::logger.info("Connected to SPI bus");
}

void spi_bus::open_from_fd(int other_fd) {
  close();
  fd = dup(other_fd);
  if (fd == -1) {
    Module::logger.error("Failed to dup SPI device");
  }
  Module::logger.info("Connected to SPI bus");
}

void spi_bus::close() {
  if (fd != -1) {
    ::close(fd);
    fd = -1;
    Module::logger.info("Disconnected from SPI bus");
  }
}

/**
 * transferByte
 *
 * Transmits a byte via the SPI device, and receives a byte
 * in response.
 *
 * Establishes a data structure, spi_ioc_transfer as defined
 * by spidev.h and loads the various members to pass the data
 * and configuration parameters to the SPI device via IOCTL
 */
uint8_t spi_bus::transferByte(uint8_t txData) {
  int ret;
  uint8_t rxData;
  struct spi_ioc_transfer spi_transfer;

  memset(&spi_transfer, 0, sizeof(spi_transfer));

  spi_transfer.tx_buf        = (unsigned long)&txData;
  spi_transfer.rx_buf        = (unsigned long)&rxData;
  spi_transfer.len           = 1;
  spi_transfer.speed_hz      = speed;
  spi_transfer.bits_per_word = BitsPerWord;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &spi_transfer);
  if (ret < 1) {
    if (Global::Debug) Module::logger.error("Unable to send SPI message");
  }

  return rxData;
}

/**
 * transferByteArray
 *
 * Transmits a byte array via the SPI device, and receives a byte
 * array in response.
 *
 * Establishes a data structure, spi_ioc_transfer as defined
 * by spidev.h and loads the various members to pass the data
 * and configuration parameters to the SPI device via IOCTL
 */
void spi_bus::transferByteArray(uint16_t len, uint8_t *txData, uint8_t *rxData) {
  int ret;
  struct spi_ioc_transfer spi_transfer;

  memset(&spi_transfer, 0, sizeof(spi_transfer));
  memset(rxData, 0, len);

  spi_transfer.tx_buf        = (unsigned long)txData;
  spi_transfer.rx_buf        = (unsigned long)rxData;
  spi_transfer.len           = len;
  spi_transfer.speed_hz      = speed;
  spi_transfer.bits_per_word = BitsPerWord;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &spi_transfer);
  if (ret < 1) {
    if (Global::Debug) Module::logger.error("Unable to send SPI message");
  }
}
