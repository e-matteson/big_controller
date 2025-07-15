#include "tspi.h"
#include "drv8311.h"

TSpi spi{&SPI, 10, {1'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
Drv8311 driver {&spi, 3};

void assert(bool condition) {
    if (!condition) {
        Serial.println("assertion failed");
        while(1) {}
    }
}

void setup() {
    Serial.begin(9600);
    spi.begin();
    driver.begin();

    driver.write(pwmg_period_register_t{1000});

    auto pwmg_ctrl = driver.read<pwmg_ctrl_register_t>();
    assert(pwmg_ctrl.has_value());
    pwmg_ctrl->pwm_en = 1;
    driver.write(pwmg_ctrl.value());
}

void loop() {
    for (uint16_t duty = 0; duty <= 1000; duty += 1) {
        driver.write(pwmg_a_duty_register_t{duty});
        driver.write(pwmg_b_duty_register_t{duty});
        driver.write(pwmg_c_duty_register_t{duty});
        delay(2);
    }
    for (uint16_t duty = 1000; duty >= 0; duty -= 1) {
        driver.write(pwmg_a_duty_register_t{duty});
        driver.write(pwmg_b_duty_register_t{duty});
        driver.write(pwmg_c_duty_register_t{duty});
        delay(2);
    }
}
