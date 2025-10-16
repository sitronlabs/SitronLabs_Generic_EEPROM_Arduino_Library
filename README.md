# Sitron Labs Generic EEPROM Arduino Library

[![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)](https://github.com/sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE.txt)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-orange.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-blue.svg)](https://platformio.org/)

A professional, easy-to-use Arduino library for various I2C EEPROM devices. This library provides comprehensive support for data storage with both direct addressing and Arduino Stream interface compatibility across multiple EEPROM types and sizes.

## Features

- **Multiple EEPROM Support**: Support for various I2C EEPROM devices and sizes
- **I2C Communication**: Simple I2C interface with multiple address options
- **Stream Interface**: Compatible with Arduino Stream class for easy data handling
- **Page Write Support**: Optimized page write operations when supported by platform
- **Automatic Write Timing**: Built-in write cycle completion detection
- **Address Validation**: Automatic bounds checking and rollover prevention
- **Professional Documentation**: Extensive inline documentation and Doxygen support
- **Error Handling**: Robust error checking and reporting
- **Maker-Friendly**: Designed with ease of use and learning in mind

## Supported Devices

| Device | Description | Memory Size | Page Size | Status |
|--------|-------------|-------------|-----------|--------|
| M24C64 | 64 Kbit I2C EEPROM | 8,192 bytes | 32 bytes | ✅ Implemented |

## Installation

### Arduino IDE Library Manager
1. Open Arduino IDE
2. Go to **Tools** → **Manage Libraries**
3. Search for "SitronLabs Generic EEPROM"
4. Click **Install**

### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps = 
    sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library
```

### Manual Installation
1. Download the latest release from [GitHub](https://github.com/sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library/releases)
2. Extract the ZIP file
3. Copy the `SitronLabs_Generic_EEPROM_Arduino_Library` folder to your Arduino `libraries` directory
4. Restart Arduino IDE

## Quick Start

### Basic Read/Write Example (M24C64)
```cpp
#include <Wire.h>
#include <m24c64.h>

m24c64 eeprom;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    /* Initialize M24C64 at I2C address 0x50 */
    eeprom.setup(Wire, 0x50);
    
    if (!eeprom.detect()) {
        Serial.println("M24C64 not found!");
        while(1);
    }
    
    Serial.println("M24C64 ready!");
}

void loop() {
    /* Write data to EEPROM */
    uint8_t data[] = "Hello, EEPROM!";
    int result = eeprom.write(0, data, sizeof(data) - 1);
    
    if (result > 0) {
        Serial.print("Written ");
        Serial.print(result);
        Serial.println(" bytes");
    }
    
    delay(1000);
    
    /* Read data back */
    uint8_t buffer[20];
    result = eeprom.read(0, buffer, sizeof(buffer));
    
    if (result > 0) {
        buffer[result] = '\0';  // Null terminate
        Serial.print("Read: ");
        Serial.println((char*)buffer);
    }
    
    delay(5000);
}
```

### Stream Interface Example
```cpp
void loop() {
    /* Use as Arduino Stream */
    eeprom.seek_write(100);  // Set write position
    eeprom.print("Temperature: ");
    eeprom.println(25.6);
    
    /* Read back using Stream interface */
    eeprom.seek_read(100);   // Set read position
    while (eeprom.available()) {
        char c = eeprom.read();
        Serial.print(c);
    }
    Serial.println();
    
    delay(2000);
}
```

## API Reference

### Initialization
```cpp
/* Setup I2C communication */
int setup(TwoWire &i2c_library, uint8_t i2c_address);

/* Detect if chip is present */
bool detect();
```

### Direct Memory Access
```cpp
/* Read data from EEPROM */
int read(uint16_t address, uint8_t* data, size_t length);

/* Write data to EEPROM */
int write(uint16_t address, const uint8_t* data, size_t length);
```

### Stream Interface Methods
```cpp
/* Stream positioning */
size_t seek_read(size_t index);
size_t seek_write(size_t index);

/* Stream I/O */
int available();
int read();
int peek();
size_t write(uint8_t data);
size_t write(const uint8_t* data, size_t length);
```

### Device Information
```cpp
/* Get EEPROM specifications */
size_t size_total_get();  // Returns 8192 bytes
size_t size_page_get();   // Returns 32 bytes
```

## Error Handling

All methods return an integer error code:
- `0` or positive: Success (number of bytes read/written)
- Negative values: Error codes

Common error codes:
- `-EINVAL`: Invalid parameter (address out of range, null pointer)
- `-EIO`: I2C communication error
- `-ETIMEDOUT`: Device not responding

## License

This library is released under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.

## Support

For support, please:
1. Check the [Issues](https://github.com/sitronlabs/SitronLabs_Generic_EEPROM_Arduino_Library/issues) page
2. Join our [Discord community](https://discord.gg/b6VzayWAMZ) for live support and discussion

---

**Sitron Labs** - Helping makers build what matters

[Website](https://sitronlabs.com) | [Store](https://www.sitronlabs.com/store) | [Discord](https://discord.gg/b6VzayWAMZ) | [GitHub](https://github.com/sitronlabs)
