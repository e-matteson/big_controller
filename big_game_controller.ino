#include "tspi.h"
#include "drv8311.h"
#include <cstdint>



TSpi spi{&SPI, 10, {5'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
Drv8311 driver {&spi, 3};

void setup() {
    Serial.begin(9600);

    spi.begin();
    driver.begin();
}

void loop() {
    demo_motor();
}

void demo_motor() {
    uint32_t pause_us = 150;
    for(uint32_t i = 0; i < UINT32_MAX; i++) {
        uint32_t start_time_us = micros();

        uint32_t a_index = i % 360;
        uint32_t b_index = (i + 120) % 360;
        uint32_t c_index = (i + 240) % 360;

        pwmg_a_duty_register_t a_reg{driver.lookup(a_index)};
        pwmg_b_duty_register_t b_reg{driver.lookup(b_index)};
        pwmg_c_duty_register_t c_reg{driver.lookup(c_index)};

        driver.write(a_reg);
        driver.write(b_reg);
        driver.write(c_reg);
        // a_reg.print();
        // b_reg.print();
        // c_reg.print();

        // TODO micros will rollover after an hour
        delayMicroseconds(start_time_us + pause_us - micros());
    }

}
