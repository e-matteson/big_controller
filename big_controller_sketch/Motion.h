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

    // void openLoopMove(float target_mdeg) {
    //     float margin_mdeg = 2.0;
    //     for (float command_edeg = 0.0; command_edeg < 720.0;
    //          command_edeg += 0.5) {
    //       m_Motor->commandElectricalAngle(normalizePosition(command_edeg), 0.5);
    //       delay(10);
    //       float pos_mdeg = m_Encoder->getSensorAngleDegrees();
    //       Serial1.println(pos_mdeg);
    //       if (pos_mdeg > target_mdeg - margin_mdeg &&
    //           pos_mdeg < target_mdeg + margin_mdeg) {
    //         Serial1.println("Found target!");
    //         return;
    //       }
    //     }
    // }

    void hardstopRoutine() {
        findHardstop(true);
        float pos_hardstop_mdeg = m_Encoder->getSensorAngleDegrees();
        findHardstop(false);
        float neg_hardstop_mdeg = m_Encoder->getSensorAngleDegrees();

        float center_mdeg = neg_hardstop_mdeg + modularDistance(pos_hardstop_mdeg, neg_hardstop_mdeg) / 2.;


    }

    std::optional<std::pair<float, float>> findHardstops() {
        // TODO rewrite this to find both hardstops. After finding the first, it
        // should start at that edeg (but take margin into account) and then
        // move in the opposite direction. Then we can move to center, and
        // probably know enough to set the offset without using the align
        // function.

        float margin_mdeg = 5.0;

        m_Motor->commandElectricalAngle(0, 0.5);
        delay(100);
        float start_mdeg = m_Encoder->getSensorAngleDegrees();
        
        for (float command_edeg = 0.0; command_edeg < 720.0;
             command_edeg += 0.5) {


          m_Motor->commandElectricalAngle(normalizePosition(command_edeg), 0.5);
          delay(10);
          float pos_mdeg = m_Encoder->getSensorAngleDegrees();
          float expected_mdeg = start_mdeg + command_edeg / m_NumPoles;

          Serial1.print("actual ");
          Serial1.print(pos_mdeg);
          Serial1.print(", expected ");
          Serial1.print(expected_mdeg);
          Serial1.print(", command ");
          Serial1.println(command_edeg);
          if (modularDistance(pos_mdeg, expected_mdeg) > margin_mdeg) {
            Serial1.println("Found hardstop!");
            return;
          }
        }
        Serial1.println("Hardstop not found");
    }

    // float getM

    void align() {
        // TODO
        Serial1.print("align");
        float initial_pos_mdeg = m_Encoder->getSensorAngleDegrees();

        m_Motor->commandElectricalAngle(0, 0.75);
        delay(1000);
        float pos_mdeg = m_Encoder->getSensorAngleDegrees();
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
        return (pos_mdeg - sectorWidth_mdeg * sectorNum) * m_NumPoles;
    }

    float getPosition_mdeg() {
        return m_Encoder->getSensorAngleDegrees() + m_EncoderOffset_mdeg;
    }

    void update() {
        float pos_mdeg = normalizePosition(getPosition_mdeg());
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

        Serial1.print(" target_mdeg: ");
        Serial1.print(m_Target_mdeg);
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
    float m_EncoderOffset_mdeg = 0;
private:
    MotorDriver* m_Motor;
    MagSensor* m_Encoder;
    const uint8_t m_NumPoles;

    float m_Target_mdeg = 90;
    float m_MaxDistance_mdeg = 30;
};
