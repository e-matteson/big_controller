#pragma once

#include "MotorDriver.h"
#include "MagSensor.h"

#include <cstdint>
class Motion {
public:
    Motion(MotorDriver* motor, MagSensor* encoder, uint8_t num_poles)
        : m_Motor(motor)
        , m_Encoder(encoder)
        , m_NumPoles(num_poles) {
    }

    void begin() {
        // TODO
    }

    uint16_t angleToIndex(float pos_mdeg) {
        int sectorWidth_mdeg = 360 / m_NumPoles;
        float pos_edeg = fmod(pos_mdeg, sectorWidth_mdeg) * m_NumPoles;
        return static_cast<uint16_t>(round(pos_edeg));
    }

    void setTarget(float pos_mdeg) {
        m_Target_mdeg = pos_mdeg;
    }

    void update() {
        float pos_mdeg = m_Encoder->getSensorAngle();
        uint16_t tableIndex = angleToIndex(pos_mdeg);

        float distance_mdeg = m_Target_mdeg - pos_mdeg;
        
        float scaleFactor = constrain(distance_mdeg / m_MaxDistance_mdeg, 0.0, 1.0) * m_MaxScaleFactor;
        m_Motor->setFromSineTable(tableIndex, scaleFactor);

        Serial.print("pos: ");
        Serial.print(pos_mdeg);
        Serial.print(" index: ");
        Serial.print(tableIndex);
        Serial.print("scale: ");
        Serial.println(scaleFactor);
    }

private:
    MotorDriver* m_Motor;
    MagSensor* m_Encoder;
    const uint8_t m_NumPoles;

    float m_Target_mdeg = 90;
    float m_MaxDistance_mdeg = 30;
    float m_MaxScaleFactor = 0.75;
};
