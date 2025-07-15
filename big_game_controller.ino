#include <SPI.h>


// Based on example from TI forum: https://e2e.ti.com/support/motor-drivers-group/motor-drivers/f/motor-drivers-forum/1434191/drv8311-application-issues

class TSpi {
public:
    struct Result {
       uint8_t status;
       uint16_t data;
    };

    TSpi(SPIClass* spi, uint8_t cs_pin, SPISettings settings)
        : m_Spi(spi), m_CsPin(cs_pin), m_Settings(settings) {
    }

    void begin() {
        pinMode(m_CsPin, OUTPUT);
        m_Spi->begin();
    }

    Result read(uint16_t id, uint16_t addr) {
        auto result = transfer(true, id, addr, 0);
        Serial.print("read from ");
        Serial.print(addr);
        Serial.print(": ");
        Serial.print(result.status);
        Serial.print(" ");
        Serial.println(result.data);
        return result;
    }

    Result write(uint16_t id, uint16_t addr, uint16_t data) {
        auto result = transfer(false, id, addr, data);
        
        Serial.print("write ");
        Serial.print(data);
        Serial.print(" to ");
        Serial.print(addr);
        Serial.print(": ");
        Serial.print(result.status);
        Serial.print(" ");
        Serial.println(result.data);
        return result;
    }

    Result transfer(bool is_read, uint16_t id, uint16_t addr, uint16_t data) {
       size_t num_bytes = 4;

       uint16_t addr_parity = parity_calc(addr);
       uint16_t data_parity = parity_calc(data);

       //   765432107654321076543210
       // 0bW  IIAAAAAAAA  PPDDDDDDD
       // 0b00011000
       uint16_t command_bytes =
           (is_read << 15) | (id << 11) | (addr << 3) | (addr_parity << 0);
       uint16_t data_bytes = (data_parity << 15) | data;

       uint8_t tx_data[num_bytes] = {
           static_cast<uint8_t>((command_bytes >> 8) & 0xFF),
           static_cast<uint8_t>(command_bytes & 0xFF),
           static_cast<uint8_t>((data_bytes >> 8) & 0xFF),
           static_cast<uint8_t>(data_bytes & 0xFF),
       };

       // Serial.print("tx_data: ");
       //  Serial.print(tx_data[0]);
       //  Serial.print(" ");
       //  Serial.print(tx_data[1]);
       //  Serial.print(" ");
       //  Serial.print(tx_data[2]);
       //  Serial.print(" ");
       //  Serial.println(tx_data[3]);

       uint8_t rx_data[num_bytes] = {0};

       digitalWrite(m_CsPin, LOW);
       m_Spi->beginTransaction(m_Settings);
       m_Spi->transfer(tx_data, rx_data, num_bytes);
       m_Spi->endTransaction();
       digitalWrite(m_CsPin, HIGH);

       return Result{
           .status = rx_data[1],
           .data = static_cast<uint16_t>((rx_data[2] << 8) | rx_data[3]),
       };
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



//cpol=0, cpha=1
TSpi spi{&SPI, 10, {1'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
const int driver_id = 3;

void printAll() {
    uint8_t addresses[] = {0x0,  0x4,  0x5,  0x6,  0x7,  0xC,  0x10,
                           0x12, 0x13, 0x16, 0x17, 0x18, 0x19, 0x1A,
                           0x1B, 0x1C, 0x1D, 0x20, 0x22, 0x23, 0x3F};

    for (auto addr : addresses) {
        spi.read(driver_id, addr);
    }
}

uint8_t flt_clr = 0x17;
uint16_t pwmg_ctrl = 0x1D;
uint16_t pwmg_period = 0x18;
uint16_t pwmg_a_duty = 25;
uint16_t pwmg_b_duty = 26;
uint16_t pwmg_c_duty = 27;

void setup() {
    spi.begin();
    Serial.begin(9600);
    spi.write(driver_id, flt_clr, 1);

    spi.write(driver_id, pwmg_period, 1000);

    spi.write(driver_id, pwmg_ctrl, 1 << 10);
}



void loop() {
    for (int duty = 0; duty <= 1000; duty += 1) {
        spi.write(driver_id, pwmg_a_duty, duty);
        spi.write(driver_id, pwmg_b_duty, duty);
        spi.write(driver_id, pwmg_c_duty, duty);
        delay(2);
    }
    for (int duty = 1000; duty >= 0; duty -= 1) {
        spi.write(driver_id, pwmg_a_duty, duty);
        spi.write(driver_id, pwmg_b_duty, duty);
        spi.write(driver_id, pwmg_c_duty, duty);
        delay(2);
    }
}
