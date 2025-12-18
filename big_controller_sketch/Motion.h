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
        Serial1.print("align");
        float initial_pos_mdeg = m_Encoder->getSensorAngle();

        m_Motor->commandElectricalAngle(0, 0.75);
        delay(1000);
        float pos_mdeg = m_Encoder->getSensorAngle();
        m_EncoderOffset_mdeg = -1 * pos_mdeg;
        Serial1.print("initial pos: ");
        Serial1.print(initial_pos_mdeg);
        Serial1.print("align pos: ");
        Serial1.print(pos_mdeg);
        Serial1.print(" offset: ");
        Serial1.print(m_EncoderOffset_mdeg);
    }

    void setTarget(float pos_mdeg) {
        m_Target_mdeg = pos_mdeg;
    }

    float mechToElecDeg(float pos_mdeg) {
        float sectorWidth_mdeg = 360.0 / m_NumPoles;
        int sectorNum = static_cast<int>(pos_mdeg / sectorWidth_mdeg);

        Serial1.print(" sector: ");
        Serial1.print(sectorNum);
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
        // Limit the max pwm to prevent overheating
        uint8_t scaleFactor = map(distanceFactor, 0, 100, 0, 50);

        m_Motor->commandElectricalAngle(command_edeg, scaleFactor/100.0);

        Serial1.print(" pos_mdeg: ");
        Serial1.print(pos_mdeg);
        Serial1.print(" pos_edeg: ");
        Serial1.print(pos_edeg);
        Serial1.print(" cmd_edeg: ");
        Serial1.print(command_edeg);
        Serial1.print(" dist_mdeg: ");
        Serial1.print(dist_mdeg);
        Serial1.print(" distanceFactor: ");
        Serial1.print(distanceFactor);
        Serial1.print(" scale: ");
        Serial1.println(scaleFactor);
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
};
