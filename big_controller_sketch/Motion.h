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

    // float getM

    void align() {
        // TODO
        Serial.print("align");
        float initial_pos_mdeg = m_Encoder->getSensorAngle();

        m_Motor->commandElectricalAngle(0, 0.75);
        delay(1000);
        float pos_mdeg = m_Encoder->getSensorAngle();
        m_EncoderOffset_mdeg = -1 * pos_mdeg;
        Serial.print("initial pos: ");
        Serial.print(initial_pos_mdeg);
        Serial.print("align pos: ");
        Serial.print(pos_mdeg);
        Serial.print(" offset: ");
        Serial.print(m_EncoderOffset_mdeg);
    }

    void setTarget(float pos_mdeg) {
        m_Target_mdeg = pos_mdeg;
    }

    float mechToElecDeg(float pos_mdeg) {
        int sectorWidth_mdeg = 360 / m_NumPoles;
        int sectorNum = pos_mdeg / sectorWidth_mdeg;

        return (pos_mdeg - sectorWidth_mdeg * sectorNum) * m_NumPoles;
    }

    void update() {
        float pos_mdeg = normalizePosition(m_Encoder->getSensorAngle() + m_EncoderOffset_mdeg);
        float pos_edeg = mechToElecDeg(pos_mdeg);

        auto forward_dist = abs(deltaForModularMove(pos_mdeg, m_Target_mdeg, true));
        auto reverse_dist = abs(deltaForModularMove(pos_mdeg, m_Target_mdeg, false));
        float dist_mdeg;
        float command_edeg;
        if (forward_dist < reverse_dist)  {
            dist_mdeg = forward_dist;
            command_edeg = normalizePosition(pos_edeg + 90);
        } else {
            dist_mdeg = reverse_dist;
            command_edeg = normalizePosition(pos_edeg - 90);
        }


        uint8_t distanceFactor = constrain(dist_mdeg / m_MaxDistance_mdeg * 100, 0, 100);
        uint8_t scaleFactor = map(distanceFactor, 0, 100, 0, 75);

        m_Motor->commandElectricalAngle(command_edeg, scaleFactor/100.0);

        Serial.print("pos_mdeg: ");
        Serial.print(pos_mdeg);
        Serial.print(" pos_edeg: ");
        Serial.print(pos_edeg);
        Serial.print(" cmd_edeg: ");
        Serial.print(command_edeg);
        Serial.print(" dist_mdeg: ");
        Serial.print(dist_mdeg);
        Serial.print(" distanceFactor: ");
        Serial.print(distanceFactor);
        Serial.print(" scale: ");
        Serial.println(scaleFactor);
    }

    static float modularDistance(float start_pos, float end_pos) {
        auto forward_dist = abs(deltaForModularMove(start_pos, end_pos, true));
        auto reverse_dist = abs(deltaForModularMove(start_pos, end_pos, false));
        return min(forward_dist, reverse_dist);
    }

    // return the position wrapped to within the range [0, 360)
    static float normalizePosition(float position) {
        return fmod(fmod(position, 360.0) + 360.0, 360.0);
    }

    static float deltaForModularMove(float start_pos, float end_pos, bool direction_positive) {
        float start_pos_mod = normalizePosition(start_pos);
        float end_pos_mod = normalizePosition(end_pos);

        if (direction_positive) {
          return fmod((end_pos_mod - start_pos_mod + 360.0), 360.0);
        } else {
          return -1 * fmod((start_pos_mod - end_pos_mod + 360.0), 360.0);
        }
    }
private:
    MotorDriver* m_Motor;
    MagSensor* m_Encoder;
    const uint8_t m_NumPoles;

    float m_EncoderOffset_mdeg = 0;
    float m_Target_mdeg = 90;
    float m_MaxDistance_mdeg = 30;
    float m_MaxScaleFactor = 0.75;
};
