/**
 * SPI Bus Class
 *
 * Written By: Chris Capobianco
 * Date: 2018-05-15
 */
#pragma once

#include <cerrno>
#include <cstring>
#include <system_error>
#include <string>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>

#include <execinfo.h>
#include <unistd.h>

class spi_bus {
public:
  explicit spi_bus(const std::string &name);
  spi_bus();

  // Move constructor
  spi_bus(spi_bus &&);

  // Copy constructor
  spi_bus(const spi_bus &);

  // Move assignment operator
  spi_bus & operator=(spi_bus &&);

  // Copy assignment operator
  spi_bus & operator=(const spi_bus &);

  ~spi_bus();

  void open(const std::string & name);
  void open_from_fd(int other_fd);
  void close();

  // Transfer Byte
	uint8_t transferByte(uint8_t txData);

	// Transfer Byte Array
	void transferByteArray(uint16_t len, uint8_t *txData, uint8_t *rxData);

  // Static Constants
  static const int SpiSpeed = 4000000;
  static const int BitsPerWord = 8;
private:
  int fd;
  unsigned int speed;
};