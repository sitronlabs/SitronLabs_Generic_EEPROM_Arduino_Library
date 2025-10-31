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

    /** @name Buffered Write Operations
     * @{
     *
     * These functions provide buffered write operations that batch multiple consecutive
     * byte writes into page write operations for improved performance. This is particularly
     * useful when you have no control over calling code that makes many small consecutive
     * writes (e.g., serializers, loggers, or protocol handlers).
     *
     * @note Buffering only occurs for writes that start at page boundaries (multiples of 32).
     *       Non-page-aligned writes are sent directly to the EEPROM. The buffer automatically
     *       flushes when it becomes full (one page), when a non-consecutive write is attempted,
     *       or when buffer_flush() is explicitly called.
     *
     * @warning Always call buffer_flush() after your last buffered_write() call to ensure
     *          all pending buffered data is written to the EEPROM before power loss or program
     *          completion.
     */

    /**
     * @brief Write data to the EEPROM using buffered page writes
     *
     * This function buffers consecutive writes that start at page boundaries and automatically
     * combines them into efficient page write operations. If the incoming address is not
     * consecutive with existing buffered data, the buffer is automatically flushed first.
     *
     * @param[in] address Starting EEPROM address to write to (0-8191)
     * @param[in] data Pointer to data buffer to write
     * @param[in] length Number of bytes to write
     * @return Number of bytes processed (buffered or written) on success, or negative error
     *         code on failure. Note: Buffered bytes are not written until the buffer is full
     *         or buffer_flush() is called.
     *
     * @note The buffer automatically flushes when it reaches one page size (32 bytes) or when
     *       a non-consecutive write is attempted.
     */
    int buffered_write(const uint16_t address, const uint8_t* const data, const size_t length);

    /**
     * @brief Flush any pending buffered writes to the EEPROM
     *
     * Forces any data currently in the write buffer to be written to the EEPROM immediately.
     * This is safe to call even if the buffer is empty.
     *
     * @return Number of bytes written on success (0 if buffer was empty), or negative error
     *         code on failure
     *
     * @note Always call this function after your last buffered_write() call to ensure all
     *       buffered data is committed to the EEPROM.
     */
    int buffer_flush(void);

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

    /** @name Device Information Methods
     * @{
     */

    /**
     * @brief Get the total size of the EEPROM in bytes
     * @return Total EEPROM size in bytes (8192 for M24C64)
     */
    static constexpr size_t size_total_get(void) {
        return m_size_total;
    }

    /**
     * @brief Get the page size for write operations in bytes
     * @return Page size in bytes (32 for M24C64)
     */
    static constexpr size_t size_page_get(void) {
        return m_size_page;
    }

    /** @} */

   protected:
    TwoWire* m_i2c_library = NULL;               /**< @brief Pointer to I2C library instance */
    uint8_t m_i2c_address;                       /**< @brief I2C address of the M24C64 device */
    size_t m_index_write = 0;                    /**< @brief Current write position in stream */
    size_t m_index_read = 0;                     /**< @brief Current read position in stream */
    static constexpr size_t m_size_total = 8192; /**< @brief Total EEPROM size in bytes (64 Kbit) */
    static constexpr size_t m_size_page = 32;    /**< @brief Page size for write operations (32 bytes) */
    uint32_t m_timestamp_write = 0;              /**< @brief Timestamp of last write operation for timing */
    uint8_t m_buffer[m_size_page];               /**< @brief Buffer for buffered writes */
    size_t m_buffer_start = 0;                   /**< @brief Index of next write position in buffer */
    size_t m_buffer_length = 0;                  /**< @brief Length of data in buffer */
};

#endif
