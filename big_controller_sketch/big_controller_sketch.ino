#include "tspi.h"
#include "motor.h"
#include "mag_sensor.h"
#include <cstdint>

#include <Wire.h>

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
