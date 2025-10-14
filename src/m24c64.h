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
 * @brief M24C64 EEPROM driver class for Arduino
 *
 * This class provides a high-level interface for communicating with M24C64
 * 64 Kbit (8 Kbyte) EEPROM devices over I2C. It inherits from the Arduino
 * Stream class, making it compatible with standard Arduino I/O functions.
 *
 * The M24C64 is a 64 Kbit serial I2C bus EEPROM organized as 8192 x 8 bits.
 * It supports both random and sequential read modes, and page write operations
 * for improved write performance.
 *
 * @note This driver supports both page write (when platform supports it) and
 *       byte-by-byte write operations with automatic write cycle timing.
 */
class m24c64 : public Stream {

   public:
    /** @name Core EEPROM Operations
     * @{
     */

    /**
     * @brief Initialize the M24C64 EEPROM driver
     * @param i2c_library Reference to the I2C library instance to use
     * @param i2c_address I2C address of the M24C64 device (0x50-0x57)
     * @return 0 on success, negative error code on failure
     */
    int setup(TwoWire& i2c_library, const uint8_t i2c_address);

    /**
     * @brief Detect if the M24C64 device is present on the I2C bus
     * @return true if device is detected, false otherwise
     */
    bool detect(void);

    /**
     * @brief Read data from the EEPROM
     * @param address Starting address to read from (0-8191)
     * @param data Pointer to buffer to store read data
     * @param length Number of bytes to read
     * @return Number of bytes read on success, negative error code on failure
     */
    int read(const uint16_t address, uint8_t* const data, const size_t length);

    /**
     * @brief Write data to the EEPROM
     * @param address Starting address to write to (0-8191)
     * @param data Pointer to data to write
     * @param length Number of bytes to write
     * @return Number of bytes written on success, negative error code on failure
     */
    int write(const uint16_t address, const uint8_t* const data, const size_t length);

    /** @} */

    /** @name Stream Interface Methods
     * @{
     */

    /**
     * @brief Get number of bytes available for reading
     * @return Number of bytes available
     */
    int available();

    /**
     * @brief Read a single byte from the stream
     * @return Byte read or -1 if no data available
     */
    int read();

    /**
     * @brief Peek at the next byte without consuming it
     * @return Next byte or -1 if no data available
     */
    int peek();

    /** @} */

    /** @name Print Interface Methods
     * @{
     */

    /**
     * @brief Write a single byte to the stream
     * @param data Byte to write
     * @return Number of bytes written
     */
    size_t write(uint8_t data);

    /**
     * @brief Write multiple bytes to the stream
     * @param data Pointer to data to write
     * @param length Number of bytes to write
     * @return Number of bytes written
     */
    size_t write(const uint8_t* data, size_t length);

    /** @} */

    /** @name Stream Positioning Methods
     * @{
     */

    /**
     * @brief Set the read position in the stream
     * @param index Position to seek to
     * @return New position or (size_t)-1 on error
     */
    size_t seek_read(size_t index);

    /**
     * @brief Set the write position in the stream
     * @param index Position to seek to
     * @return New position or (size_t)-1 on error
     */
    size_t seek_write(size_t index);

    /** @} */

   protected:
    TwoWire* m_i2c_library = NULL;    /**< @brief Pointer to I2C library instance */
    uint8_t m_i2c_address;            /**< @brief I2C address of the M24C64 device */
    size_t m_index_write = 0;         /**< @brief Current write position in stream */
    size_t m_index_read = 0;          /**< @brief Current read position in stream */
    const size_t m_size_total = 8192; /**< @brief Total EEPROM size in bytes (64 Kbit) */
    const size_t m_size_page = 32;    /**< @brief Page size for write operations (32 bytes) */
    uint32_t m_timestamp_write = 0;   /**< @brief Timestamp of last write operation for timing */
};

#endif
