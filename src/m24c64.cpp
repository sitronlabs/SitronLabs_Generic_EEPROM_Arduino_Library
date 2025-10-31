/* Self header */
#include "m24c64.h"

/* Determine length of i2c transactions supported by the platform */
#if defined(WIRE_BUFFER_SIZE)
#define PAGE_WRITE_SUPPORTED 1
#define I2C_BUFFER_SIZE WIRE_BUFFER_SIZE
#else
#warning "Platform doesn't support page write"
#define PAGE_WRITE_SUPPORTED 0
#endif

/**
 * Configures the driver with access over I2C.
 * @note Call this from the Arduino setup function.
 * @note Make sure the I2C library has been initialized with a call to its begin function for example.
 * @param[in] i2c_library A reference to the i2c library to use.
 * @param[in] i2c_address The i2c address of the device.
 * @return 0 in case of success, or a negative error code otherwise.
 */
int m24c64::setup(TwoWire& i2c_library, const uint8_t i2c_address) {

    /* Ensure I2C address is within valid range (0x50-0x57) */
    if ((i2c_address & 0xF8) != 0x50) {
        return -EINVAL;
    }

    /* Store I2C library reference and device address */
    m_i2c_library = &i2c_library;
    m_i2c_address = i2c_address;

    /* Return success */
    return 0;
}

/**
 * Tries to detect the device.
 * @return true if the device has been detected, or false otherwise.
 */
bool m24c64::detect(void) {
    if (m_i2c_library != NULL) {
        m_i2c_library->beginTransmission(m_i2c_address);
        if (m_i2c_library->endTransmission() == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Read data from the M24C64 EEPROM
 * @param[in] address Starting address to read from (0-8191)
 * @param[out] data Buffer to store the read data
 * @param[in] length Number of bytes to read
 * @return The number of bytes read in case of success, or a negative error code otherwise.
 */
int m24c64::read(const uint16_t address, uint8_t* const data, const size_t length) {
    int res;

    /* Ensure setup has been performed */
    if (m_i2c_library == NULL) {
        return -EINVAL;
    }

    /* Ensure start address is valid and prevent address rollover */
    if (address >= m_size_total) {
        return -EINVAL;
    }
    size_t length_capped = (address + length > m_size_total) ? m_size_total - address : length;

    /* Wait for write cycle completion if a write was recently performed */
    while ((millis() - m_timestamp_write) <= 5) {
        if (detect() == true) {
            break;
        }
    }

    /* Read bytes from EEPROM */
    for (size_t i = 0; i < length_capped;) {
        m_i2c_library->beginTransmission(m_i2c_address);
        m_i2c_library->write((uint8_t)((address + i) >> 8));
        m_i2c_library->write((uint8_t)((address + i) >> 0));
        if (m_i2c_library->endTransmission(false) != 0) return -EIO;
        res = m_i2c_library->requestFrom(m_i2c_address, length_capped - i);
        if (res <= 0) {
            return i;
        } else {
            for (size_t j = 0; j < ((unsigned int)res); j++) {
                data[i + j] = m_i2c_library->read();
            }
            i += res;
        }
    }

    /* Return number of bytes successfully read */
    return length_capped;
}

/**
 * @brief Write data to the M24C64 EEPROM
 * @param[in] address Starting address to write to (0-8191)
 * @param[in] data Data to write to the EEPROM
 * @param[in] length Number of bytes to write
 * @return The number of bytes written in case of success, or a negative error code otherwise.
 */
int m24c64::write(const uint16_t address, const uint8_t* const data, const size_t length) {

    /* Ensure setup has been performed */
    if (m_i2c_library == NULL) {
        return -EINVAL;
    }

    /* Ensure start address is valid and prevent address rollover */
    if (address >= m_size_total) {
        return -EINVAL;
    }
    size_t length_capped = (address + length > m_size_total) ? m_size_total - address : length;

    /* Write bytes to EEPROM */
    for (size_t i = 0; i < length_capped;) {

        /* Wait for previous write cycle completion if needed */
        while ((millis() - m_timestamp_write) <= 5) {
            if (detect() == true) {
                break;
            }
        }

#if PAGE_WRITE_SUPPORTED
        /* Use page write when possible for better performance */
        if (((address + i) % m_size_page == 0) && (length_capped - i >= m_size_page) && (I2C_BUFFER_SIZE >= m_size_page + 2)) {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write((uint8_t)((address + i) >> 8));
            m_i2c_library->write((uint8_t)((address + i) >> 0));
            i += m_i2c_library->write(&data[i], m_size_page);
            if (m_i2c_library->endTransmission(true) != 0) return -EIO;
            m_timestamp_write = millis();
        }

        /* Fall back to byte-by-byte write */
        else {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write((uint8_t)((address + i) >> 8));
            m_i2c_library->write((uint8_t)((address + i) >> 0));
            i += m_i2c_library->write(data[i]);
            if (m_i2c_library->endTransmission(true) != 0) return -EIO;
            m_timestamp_write = millis();
        }
#else
        /* Use byte-by-byte write when page write is not supported */
        m_i2c_library->beginTransmission(m_i2c_address);
        m_i2c_library->write((uint8_t)((address + i) >> 8));
        m_i2c_library->write((uint8_t)((address + i) >> 0));
        i += m_i2c_library->write(data[i]);
        if (m_i2c_library->endTransmission(true) != 0) return -EIO;
        m_timestamp_write = millis();
#endif
    }

    /* Return number of bytes successfully written */
    return length_capped;
}

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
int m24c64::buffered_write(const uint16_t address, const uint8_t* const data, const size_t length) {
    int res;

    /* Ensure setup has been performed */
    if (m_i2c_library == NULL) {
        return -EINVAL;
    }

    /* Ensure start address is valid and prevent address rollover */
    if (address >= m_size_total) {
        return -EINVAL;
    }
    size_t length_capped = (address + length > m_size_total) ? m_size_total - address : length;

#if PAGE_WRITE_SUPPORTED
    /* If the data is not contiguous with the buffer, flush the buffer first */
    if (address != (m_buffer_start + m_buffer_length)) {
        res = buffer_flush();
        if (res < 0) {
            return res;
        }
    }

    /* For each byte */
    for (size_t i = 0; i < length_capped; i++) {

        /* If the buffer already has data, append to it */
        if (m_buffer_length > 0) {
            m_buffer[m_buffer_length] = data[i];
            m_buffer_length++;

            /* If the buffer is full, flush it */
            if (m_buffer_length >= m_size_page) {
                int res = buffer_flush();
                if (res < 0) {
                    return res;
                }
            }
        }

        /* If the buffer is empty */
        else {

            /* Start a new buffer only if it aligns with a page start */
            if ((address + i) % m_size_page == 0) {
                m_buffer_start = address + i;
                m_buffer[m_buffer_length] = data[i];
                m_buffer_length++;
            }

            /* Otherwise, write byte normally */
            else {
                res = write(address + i, &data[i], 1);
                if (res < 0) {
                    return res;
                }
            }
        }
    }

    /* Return number of bytes successfully processed */
    return length_capped;
#else
    /* Page write not supported, forward to regular write function */
    return write(address, data, length);
#endif
}

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
int m24c64::buffer_flush(void) {
    int res;

    /* If buffer is not empty, write it to the EEPROM */
    if (m_buffer_length > 0) {
        res = write(m_buffer_start, m_buffer, m_buffer_length);
        if (res < 0) {
            return res;
        }

        /* Reset buffer */
        m_buffer_length = 0;
    } else {
        res = 0;
    }

    /* Return number of bytes written */
    return res;
}

/**
 * Gets the number of bytes available in the stream. This is only for bytes that have already arrived.
 * @note Inherited from the stream interface
 * @see https://docs.arduino.cc/language-reference/en/functions/communication/stream/streamAvailable/
 */
int m24c64::available() {
    if (m_index_read <= m_size_total) {
        return m_size_total - m_index_read;
    } else {
        return 0;
    }
}

/**
 * Reads characters from an incoming stream to the buffer.
 * @note Inherited from the stream interface
 * @see https://docs.arduino.cc/language-reference/en/functions/communication/stream/streamRead/
 */

int m24c64::read() {
    uint8_t data;
    int res = read(m_index_read, &data, 1);
    if (res < 0) {
        return -1;
    } else {
        m_index_read += res;
        return data;
    }
}

/**
 * Read a byte from the file without advancing to the next one. That is, successive calls to peek() will return the same value, as will the next call to read().
 * @note Inherited from the stream interface
 * @see https://docs.arduino.cc/language-reference/en/functions/communication/stream/streamPeek/
 */
int m24c64::peek() {
    uint8_t data;
    int res = read(m_index_read, &data, 1);
    if (res < 0) {
        return -1;
    } else {
        return data;
    }
}

/**
 * @brief Write a single byte to the stream
 * @param[in] data Byte to write
 * @return Number of bytes written
 * @note Inherited from the print interface
 */
size_t m24c64::write(uint8_t data) {
    int res = write(m_index_write, &data, 1);
    if (res < 0) {
        return 0;
    } else {
        m_index_write += res;
        return res;
    }
}

/**
 * @brief Write multiple bytes to the stream
 * @param[in] data Pointer to data to write
 * @param[in] length Number of bytes to write
 * @return Number of bytes written
 * @note Inherited from the print interface
 */
size_t m24c64::write(const uint8_t* data, size_t length) {
    int res = write(m_index_write, data, length);
    if (res < 0) {
        return 0;
    } else {
        m_index_write += res;
        return res;
    }
}

/**
 * @brief Set the read position in the stream
 * @param index Position to seek to (0-8191)
 * @return New position or (size_t)-1 on error
 */
size_t m24c64::seek_read(size_t index) {
    if (index < m_size_total) {
        m_index_read = index;
        return index;
    } else {
        return ((size_t)-1);
    }
}

/**
 * @brief Set the write position in the stream
 * @param index Position to seek to (0-8191)
 * @return New position or (size_t)-1 on error
 */
size_t m24c64::seek_write(size_t index) {
    if (index < m_size_total) {
        m_index_write = index;
        return index;
    } else {
        return ((size_t)-1);
    }
}
