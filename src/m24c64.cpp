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

    /* Ensure i2c address is within valid range */
    if ((i2c_address & 0xF8) != 0x50) {
        return -EINVAL;
    }

    /* Enable i2c */
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
 *
 * @param[in] address
 * @param[out] data
 * @param[in] length
 * @return The number of bytes read in case of success, or a negative error code otherwise.
 */
int m24c64::read(const uint16_t address, uint8_t* const data, const size_t length) {
    int res;

    /* Ensure setup has been performed */
    if (m_i2c_library == NULL) {
        return -EINVAL;
    }

    /* Ensure start address is valid and prevent stop address from creatign a rollover */
    if (address >= m_size_total) {
        return -EINVAL;
    }
    size_t length_capped = (address + length > m_size_total) ? m_size_total - address : length;

    /* Wait a little bit if a write has just been performed
     * @todo Improve by implementing polling on ack */
    while (millis() - m_timestamp_write <= 5) {
    }

    /* Read bytes */
    for (size_t i = 0; i < length_capped;) {
        m_i2c_library->beginTransmission(m_i2c_address);
        m_i2c_library->write((uint8_t)((address + i) >> 8));
        m_i2c_library->write((uint8_t)((address + i) >> 0));
        if (m_i2c_library->endTransmission(false) != 0) return -EIO;
        res = m_i2c_library->requestFrom(m_i2c_address, length_capped - i);
        if (res <= 0) {
            return i;
        } else {
            for (size_t j = 0; j < ((unsigned int) res); j++) {
                data[i + j] = m_i2c_library->read();
            }
            i += res;
        }
    }

    /* Return number of bytes read */
    return length_capped;
}

/**
 *
 * @param[in] address
 * @param[in] data
 * @param[in] length
 * @return The number of bytes written in case of success, or a negative error code otherwise.
 */
int m24c64::write(const uint16_t address, const uint8_t* const data, const size_t length) {

    /* Ensure setup has been performed */
    if (m_i2c_library == NULL) {
        return -EINVAL;
    }

    /* Ensure start address is valid and prevent stop address from creating a rollover */
    if (address >= m_size_total) {
        return -EINVAL;
    }
    size_t length_capped = (address + length > m_size_total) ? m_size_total - address : length;

    /* Write bytes */
    for (size_t i = 0; i < length_capped;) {

        /* Wait a little bit if a write has just been performed
         * @todo Improve by implementing polling on ack */
        while (millis() - m_timestamp_write <= 5) {
        }

#if PAGE_WRITE_SUPPORTED
        /* Page write when possible */
        if ((address + i) % m_size_page == 0 && length_capped - i >= m_size_page && I2C_BUFFER_SIZE >= m_size_page + 2) {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write((uint8_t)((address + i) >> 8));
            m_i2c_library->write((uint8_t)((address + i) >> 0));
            i += m_i2c_library->write(&data[i], m_size_page);
            if (m_i2c_library->endTransmission(true) != 0) return -EIO;
            m_timestamp_write = millis();
        }

        /* Byte write otherwise */
        else {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write((uint8_t)((address + i) >> 8));
            m_i2c_library->write((uint8_t)((address + i) >> 0));
            i += m_i2c_library->write(data[i]);
            if (m_i2c_library->endTransmission(true) != 0) return -EIO;
            m_timestamp_write = millis();
        }
#else
        /* Byte write */
        m_i2c_library->beginTransmission(m_i2c_address);
        m_i2c_library->write((uint8_t)((address + i) >> 8));
        m_i2c_library->write((uint8_t)((address + i) >> 0));
        i += m_i2c_library->write(data[i]);
        if (m_i2c_library->endTransmission(true) != 0) return -EIO;
        m_timestamp_write = millis();
#endif
    }

    /* Return number of bytes written */
    return length_capped;
}

/**
 * Gets the number of bytes available in the stream. This is only for bytes that have already arrived.
 * @note Inherited from the stream interface
 * @see https://www.arduino.cc/reference/en/language/functions/communication/stream/streamavailable/
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
 * @see https://www.arduino.cc/reference/en/language/functions/communication/stream/streamread
 */

int m24c64::read() {
    uint8_t data;
    int res = read(m_index_read, &data, 1);
    if (res < 0) {
        return 0;
    } else {
        m_index_read += res;
        return data;
    }
}

/**
 * Read a byte from the file without advancing to the next one. That is, successive calls to peek() will return the same value, as will the next call to read().
 * @note Inherited from the stream interface
 * @see https://www.arduino.cc/reference/en/language/functions/communication/stream/streampeek/
 */
int m24c64::peek() {
    uint8_t data;
    int res = read(m_index_read, &data, 1);
    if (res < 0) {
        return 0;
    } else {
        return data;
    }
}

/**
 *
 * @param[in] data
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
 *
 * @param[in] data
 * @param[in] length
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
 * @brief
 * @param index
 * @return
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
 * @brief
 * @param index
 * @return
 */
size_t m24c64::seek_write(size_t index) {
    if (index < m_size_total) {
        m_index_write = index;
        return index;
    } else {
        return ((size_t)-1);
    }
}
