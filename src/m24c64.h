#ifndef M24C64_H
#define M24C64_H

/* Arduino libraries */
#include <Arduino.h>
#include <Stream.h>
#include <Wire.h>

/* C/C++ libraries */
#include <errno.h>
#include <stdint.h>

/**
 *
 */
class m24c64 : public Stream {

   public:
    int setup(TwoWire& i2c_library, const uint8_t i2c_address);
    bool detect(void);
    int read(const uint16_t address, uint8_t* const data, const size_t length);
    int write(const uint16_t address, const uint8_t* const data, const size_t length);

    /* Inherited from the stream interface */
    int available();
    int read();
    int peek();

    /* Inherited from the print interface */
    size_t write(uint8_t data);
    size_t write(const uint8_t* data, size_t length);

    /* Seek for stream and print interfaces */
    size_t seek_read(size_t index);
    size_t seek_write(size_t index);

   protected:
    TwoWire* m_i2c_library = NULL;
    uint8_t m_i2c_address;
    size_t m_index_write = 0;
    size_t m_index_read = 0;
    const size_t m_size_total = 8192;  // 64 Kbit (8 Kbyte)
    const size_t m_size_page = 32;     // 32 byte
    uint32_t m_timestamp_write = 0;
};

#endif
