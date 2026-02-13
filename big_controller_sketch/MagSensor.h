#pragma once

#include <SimpleFOC.h>
#include <Wire.h>
#include <cmath>

// Version 2.0.0 works! 1.0.3 does not.
#include <SparkFun_TMAG5273_Arduino_Library.h>
#include <SparkFun_TMAG5273_Arduino_Library_Defs.h>


#include "utils.h"

class MagSensor : public Sensor {
public:
    MagSensor(TwoWire &wirePort, uint8_t address, float angle_offset)
        : Sensor()
        , m_AngleOffset(angle_offset)
        , m_I2cAddress(address)
        , m_WirePort(wirePort) {
    }

    bool begin() {
        // TODO are the return values actually right?
        if (auto rc = m_Sensor.begin(m_DefaultI2cAddress, m_WirePort); rc != 1) {
            Serial1.println("begin failed");
            return false;
        }
        delay(100);
        if (auto rc = m_Sensor.setI2CAddressEN(true); rc != 0) {
            Serial1.println("setI2CAddressEN failed");
            return false;
        }
        delay(100);
        if (auto rc = m_Sensor.setI2CAddress(m_I2cAddress); rc != 0) {
            Serial1.println("setI2CAddress failed");
            return false;
        }
        delay(100);
        if (auto rc = m_Sensor.setAngleEn(1); rc != 0) {
            Serial1.println("setAngleEn failed");
            return false;
        }
        return true;
    }


    // Get current shaft angle from the sensor hardware, and
    // return it as a float in radians, in the range 0 to 2PI.
    float getSensorAngle() override {
        float radians = getSensorAngleDegrees() * 2.0f * M_PI / 360.0f;
        return radians;
    }

    // Get current shaft angle from the sensor hardware, and
    // return it as a float in degrees, in the range 0 to 360.
    float getSensorAngleDegrees() {
        return m_Sensor.getAngleResult() + m_AngleOffset;
    }

protected:
    float m_AngleOffset;
    uint8_t m_I2cAddress;
    uint8_t m_DefaultI2cAddress = 0x78;
    TwoWire& m_WirePort;
    TMAG5273 m_Sensor;
};
