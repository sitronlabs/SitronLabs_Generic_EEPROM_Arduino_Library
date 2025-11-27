[![Designed by Sitron Labs](https://img.shields.io/badge/Designed_by-Sitron_Labs-FCE477.svg)](https://www.sitronlabs.com/)
[![Join the Discord community](https://img.shields.io/discord/552242187665145866.svg?logo=discord&logoColor=white&label=Discord&color=%237289da)](https://discord.gg/btnVDeWhfW)
[![PayPal Donate](https://img.shields.io/badge/PayPal-Donate-00457C.svg?logo=paypal&logoColor=white)](https://www.paypal.com/donate/?hosted_button_id=QLX8VU9Q3PFFL)
![License](https://img.shields.io/github/license/sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library.svg)
![Latest Release](https://img.shields.io/github/release/sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library.svg)
[![Arduino Library Manager](https://www.ardu-badge.com/badge/Sitron%20Labs%20EEPROM%20Arduino%20Library.svg)](https://www.ardu-badge.com/Sitron%20Labs%20EEPROM%20Arduino%20Library)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/sitronlabs/library/Sitron_Labs_EEPROM_Arduino_Library.svg)](https://registry.platformio.org/libraries/sitronlabs/Sitron_Labs_EEPROM_Arduino_Library)

# Sitron Labs Generic EEPROM Arduino Library

Arduino library for interfacing with various I2C EEPROM devices.

## Description

This library provides a simple and consistent interface to read and write data to I2C EEPROM devices. It supports both direct memory access and Arduino Stream interface compatibility, making it easy to use EEPROMs for data storage in your Arduino projects. The library includes automatic write cycle timing, address validation, and optimized page write operations when supported by the device and platform.

## Installation

### Arduino IDE

Install via the Arduino Library Manager by searching for "Sitron Labs EEPROM".

Alternatively, install manually:
1. Download or clone this repository
2. Place it in your Arduino `libraries` folder
3. Restart the Arduino IDE

### PlatformIO

Install via the PlatformIO Library Manager by searching for "Sitron Labs EEPROM".

Alternatively, add the library manually to your `platformio.ini` file:

```ini
lib_deps = 
    https://github.com/sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library.git
```

## Supported Devices

| Device | Description | Memory Size | Page Size | Status |
|--------|-------------|-------------|-----------|--------|
| M24C64 | 64 Kbit I2C EEPROM | 8,192 bytes | 32 bytes | ✅ Implemented |

## API Reference

### setup(TwoWire &i2c_library, uint8_t i2c_address)

Initializes the EEPROM device.

- `i2c_library`: I2C library instance to use (typically `Wire`)
- `i2c_address`: I2C address of the EEPROM device

Returns 0 on success, or a negative error code otherwise.

### detect(void)

Detects if the EEPROM device is present on the I2C bus.

Returns true if device is detected, false otherwise.

### read(uint16_t address, uint8_t* data, size_t length)

Reads data from the EEPROM.

- `address`: Starting address to read from
- `data`: Pointer to buffer where read data will be stored
- `length`: Number of bytes to read

Returns the number of bytes read on success, or a negative error code otherwise.

### write(uint16_t address, const uint8_t* data, size_t length)

Writes data to the EEPROM.

- `address`: Starting address to write to
- `data`: Pointer to buffer containing data to write
- `length`: Number of bytes to write

Returns the number of bytes written on success, or a negative error code otherwise.

### Stream Interface Methods

The EEPROM classes inherit from Arduino's Stream class, providing the following methods:

- `seek_read(size_t index)`: Sets the read position for Stream operations
- `seek_write(size_t index)`: Sets the write position for Stream operations
- `available()`: Returns the number of bytes available to read
- `read()`: Reads a byte from the EEPROM
- `peek()`: Peeks at the next byte without removing it
- `write(uint8_t data)`: Writes a byte to the EEPROM
- `print()`: Prints data to the EEPROM
- `println()`: Prints data with a newline

### Device Information Methods

- `size_total_get()`: Returns the total memory size in bytes
- `size_page_get()`: Returns the page size in bytes

## Error Handling

All methods return an integer error code:
- `0` or positive: Success (number of bytes read/written)
- Negative values: Error codes

Common error codes:
- `-EINVAL`: Invalid parameter (address out of range, null pointer)
- `-EIO`: I2C communication error
- `-ETIMEDOUT`: Device not responding

## Specifications

- Communication interface: I2C
- Stream interface: Compatible with Arduino Stream class
- Page write support: Optimized when supported by device and platform
- Automatic write timing: Built-in write cycle completion detection
- Address validation: Automatic bounds checking and rollover prevention
