#pragma once

#include <SimpleFOC.h>
#include <Wire.h>

// There's a bug in readRegister, change 2 to 1
// (https://github.com/sparkfun/SparkFun_TMAG5273_Arduino_Library/issues/11)
// Some other compiler warnings needed to be fixed, too
#include <SparkFun_TMAG5273_Arduino_Library.h>
#include <SparkFun_TMAG5273_Arduino_Library_Defs.h>


#include "utils.h"

class MagSensor : public Sensor {
public:
    MagSensor(TwoWire &wirePort)
        : Sensor()
        , m_WirePort(wirePort) {
    }

    bool begin() {
        // TODO are the return values actually right?
        if (auto rc = m_Sensor.begin(m_I2cAddress, m_WirePort); rc != 1) {
            return false;
        }
        if (auto rc = m_Sensor.setAngleEn(1); rc != 0) {
            return false;
        }
        return true;
    }

protected:
    // Get current shaft angle from the sensor hardware, and
    // return it as a float in radians, in the range 0 to 2PI.
    float getSensorAngle() override {
        return m_Sensor.getAngleResult();
    }

    const uint8_t m_I2cAddress = 0x78;
    TwoWire& m_WirePort;
    TMAG5273 m_Sensor;
};
