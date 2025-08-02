#include "tspi.h"
#include "motor.h"
#include "mag_sensor.h"
#include <cstdint>

#include <Wire.h>

int bumper_l = 0; // PA22
int drv_sleep_n = 1; // PA23
int io_expander_int_n = 2; // PA10
int button_extra = 3; // PB10
int button_a = 4; // PB11
int button_x = 5; // PA19
int dpad_left = 6; // PA02
int button_start = 7; // PB02
int button_hat_l = 8; // PA18
int button_back = 9; // PA03
int button_hat_r = 10; // PA12
int button_y = 11; // PA13
int dpad_up = 12; // PA27
int usr_led_g = 13; // PA09
int usr_led_r = 14; // PA08
int dpad_right = 15; // PA28
int button_home = 16; // PB08
int dpad_down = 17; // PB09
int bumper_r = 18; // PA00,
int button_b = 19; // PA01
int led_clk = 20; // PA20
int led_data = 21; // PA21

int ana_trigger_r = 22; // PA11
int ana_trigger_l = 23; // PB03

TSpi spi{&SPI, 10, {5'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
Motor motor {&spi, 3};

MagSensor sensor {Wire};

void setup() {
    Serial.begin(9600);

    spi.begin();
    motor.init();

    Wire.begin();
    if (!sensor.begin()) {
        Serial.println("failed to initialize tmag5723");
    }
}

void loop() {
    sensor.update();
    Serial.print("angle=");
    Serial.println(sensor.getAngle());
    delay(1000);
}

void demo_motor() {
    uint32_t pause_us = 150;
    for(uint32_t i = 0; i < UINT32_MAX; i++) {
        uint32_t start_time_us = micros();
        motor.setFromSineTable(i);
        // TODO micros will rollover after an hour
        delayMicroseconds(start_time_us + pause_us - micros());
    }

}
