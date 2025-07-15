#pragma once

#include <SPI.h>

class TSpi {
public:
    static constexpr int NUM_READ_BYTES = 3;

    TSpi(SPIClass* spi, uint8_t cs_pin, SPISettings settings)
        : m_Spi(spi), m_CsPin(cs_pin), m_Settings(settings) {
    }

    void begin() {
        pinMode(m_CsPin, OUTPUT);
        m_Spi->begin();
    }

    std::array<uint8_t, NUM_READ_BYTES> read(uint16_t id, uint16_t addr) {
        return transfer(true, id, addr, 0);
    }

    std::array<uint8_t, NUM_READ_BYTES> write(uint16_t id, uint16_t addr, uint16_t data) {
        return transfer(false, id, addr, data);
    }

    std::array<uint8_t, NUM_READ_BYTES> transfer(bool is_read, uint16_t id, uint16_t addr, uint16_t data) {

       uint16_t addr_parity = parity_calc(addr);
       uint16_t data_parity = parity_calc(data);

       uint16_t command_bytes =
           (is_read << 15) | (id << 11) | (addr << 3) | (addr_parity << 0);
       uint16_t data_bytes = (data_parity << 15) | data;

       uint8_t tx_data[NUM_READ_BYTES+1] = {
           static_cast<uint8_t>((command_bytes >> 8) & 0xFF),
           static_cast<uint8_t>(command_bytes & 0xFF),
           static_cast<uint8_t>((data_bytes >> 8) & 0xFF),
           static_cast<uint8_t>(data_bytes & 0xFF),
       };
       uint8_t rx_data[NUM_READ_BYTES+1] = {0};

       digitalWrite(m_CsPin, LOW);
       m_Spi->beginTransaction(m_Settings);
       m_Spi->transfer(tx_data, rx_data, NUM_READ_BYTES+1);
       m_Spi->endTransaction();
       digitalWrite(m_CsPin, HIGH);
       std::array<uint8_t, NUM_READ_BYTES> rx_data_out = {rx_data[1], rx_data[2], rx_data[3]};
       return rx_data_out;
    }

    bool parity_calc(uint16_t word) {
       uint16_t parity = 0;
       while (word) {
           parity ^= (word & 1);
           word >>= 1;
       }
       return parity;
    }

private:

    SPIClass* m_Spi;
    uint8_t m_CsPin;
    SPISettings m_Settings;
};
