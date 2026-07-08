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

    enum BlockingMoveResult {
        Hardstop,
        ReachedTarget,
        GaveUp
    };

    // Returns the move result and the last command in edeg
    // Distance, step and margin should be positive.
    std::pair<BlockingMoveResult, float>
    blockingMove(std::optional<float> distance_edeg, bool direction,
                 float start_command_edeg, float step_edeg, float scale_factor,
                 std::optional<float> deviation_threshold_mdeg) {
      m_Motor->commandElectricalAngle(start_command_edeg, scale_factor);
      delay(300);
      float start_mdeg = m_Encoder->getSensorAngleDegrees();


      const float full_swing_edeg = 720.;
      float command_edeg = start_command_edeg;
      int log_counter = 0;
      while (true) {
        m_Motor->commandElectricalAngle(normalizePosition(command_edeg),
                                        scale_factor);
        delay(2);
        float position_mdeg = m_Encoder->getSensorAngleDegrees();
        float expected_mdeg = start_mdeg + (command_edeg - start_command_edeg) / m_NumPoles;

        if (log_counter % 100 == 0) {
          Serial1.print("actual ");
          Serial1.print(position_mdeg);
          Serial1.print(", expected ");
          Serial1.print(expected_mdeg);
          Serial1.print(", command ");
          Serial1.println(command_edeg);
        }
        log_counter++;
        if (distance_edeg.has_value() && abs(command_edeg - start_command_edeg) >= distance_edeg.value()) {
          Serial1.print("actual ");
          Serial1.print(position_mdeg);
          Serial1.print(", expected ");
          Serial1.print(expected_mdeg);
          Serial1.print(", command ");
          Serial1.println(command_edeg);
            return std::pair(BlockingMoveResult::ReachedTarget, command_edeg);
        }
        if (deviation_threshold_mdeg.has_value()
            && modularDistance(position_mdeg, expected_mdeg) > deviation_threshold_mdeg.value()) {
          Serial1.print("actual ");
          Serial1.print(position_mdeg);
          Serial1.print(", expected ");
          Serial1.print(expected_mdeg);
          Serial1.print(", command ");
          Serial1.println(command_edeg);
            return std::pair(BlockingMoveResult::Hardstop, command_edeg);
        }
        if (direction) {
          command_edeg += step_edeg;
          if (command_edeg > start_command_edeg + full_swing_edeg) {
          Serial1.print("actual ");
          Serial1.print(position_mdeg);
          Serial1.print(", expected ");
          Serial1.print(expected_mdeg);
          Serial1.print(", command ");
          Serial1.println(command_edeg);
            return std::pair(BlockingMoveResult::GaveUp, command_edeg);
          }
        } else {
          command_edeg -= step_edeg;
          if (command_edeg < start_command_edeg - full_swing_edeg) {
          Serial1.print("actual ");
          Serial1.print(position_mdeg);
          Serial1.print(", expected ");
          Serial1.print(expected_mdeg);
          Serial1.print(", command ");
          Serial1.println(command_edeg);
            return std::pair(BlockingMoveResult::GaveUp, command_edeg);
          }
        }
      }
      // unreachable
      return std::pair(BlockingMoveResult::GaveUp, 0.0);
    }

    // Find the positive and negative hardstops, then move to the center between them.
    // On success, returns the edeg command corresponding to the center.
    std::optional<float> findCenter(float initial_command_edeg) {
        // TODO rewrite this to find both hardstops. After finding the first, it
        // should start at that edeg (but take margin into account) and then
        // move in the opposite direction. Then we can move to center, and
        // probably know enough to set the offset without using the align
        // function.
        const float step_edeg = 0.5;
        const float scale_factor = 0.75;
        const float deviation_threshold_mdeg = 4.0;
        const float backoff_dist_edeg = 30.; // TODO convert from deviation_threshold
        auto [pos_result, pos_hardstop_edeg] =
            blockingMove(std::nullopt, true, initial_command_edeg, step_edeg, scale_factor, deviation_threshold_mdeg);
        if (pos_result != BlockingMoveResult::Hardstop) {
            Serial1.println("Positive hardstop not found");
            return std::nullopt;
        }
        float pos_hardstop_mdeg = m_Encoder->getSensorAngleDegrees();
        Serial1.print("Positive hardstop found: ");
        Serial1.println(pos_hardstop_mdeg);

        auto [pos_backoff_result, pos_backoff_edeg] =
            blockingMove({backoff_dist_edeg}, false, pos_hardstop_edeg, step_edeg, scale_factor, 5.*deviation_threshold_mdeg);
        if (pos_backoff_result != BlockingMoveResult::ReachedTarget) {
            Serial1.print("Failed to back off from positive hardstop: ");
            Serial1.println(pos_backoff_result);
            return std::nullopt;
        }
        Serial1.println("positive backoff done");
        delay(100);
        auto [neg_result, neg_hardstop_edeg] =
            blockingMove(std::nullopt, false, pos_backoff_edeg, step_edeg, scale_factor, deviation_threshold_mdeg);
        if (neg_result != BlockingMoveResult::Hardstop) {
            Serial1.println("Negative hardstop not found");
            return std::nullopt;
        }
        float neg_hardstop_mdeg = m_Encoder->getSensorAngleDegrees();
        Serial1.print("Negative hardstop found: ");
        Serial1.println(neg_hardstop_mdeg);
        delay(100);

        Serial1.println("backoff done");
        delay(100);
        float distance_to_center_edeg = (pos_hardstop_edeg - neg_hardstop_edeg) / 2.;
        Serial1.print("Move to center: ");
        Serial1.println(distance_to_center_edeg);
        auto [center_result, center_edeg] =
            blockingMove({distance_to_center_edeg}, true, neg_hardstop_edeg, step_edeg, scale_factor, 5.*deviation_threshold_mdeg);
        if (center_result != BlockingMoveResult::ReachedTarget) {
            Serial1.print("Could not move to center after finding hardstops: ");
            Serial1.println(center_result);
            return std::nullopt;
        }
        return {center_edeg};
    }
    void align(float command_edeg) {
        Serial1.print("align to ");
        Serial1.print(command_edeg);
        float initial_pos_mdeg = m_Encoder->getSensorAngleDegrees();

        m_Motor->commandElectricalAngle(command_edeg, 0.75);
        delay(300);
        float pos_mdeg = m_Encoder->getSensorAngleDegrees();
        m_EncoderOffset_mdeg = -1 * pos_mdeg;
        Serial1.print("initial pos: ");
        Serial1.print(initial_pos_mdeg);
        Serial1.print(", align pos: ");
        Serial1.print(pos_mdeg);
        Serial1.print(", offset: ");
        Serial1.println(m_EncoderOffset_mdeg);
    }

    void align_zero() {
        Serial1.print("align to zero");
        float initial_pos_mdeg = m_Encoder->getSensorAngleDegrees();

        m_Motor->commandElectricalAngle(0, 0.75);
        delay(1000);
        float pos_mdeg = m_Encoder->getSensorAngleDegrees();
        m_EncoderOffset_mdeg = -1 * pos_mdeg;
        Serial1.print("initial pos: ");
        Serial1.print(initial_pos_mdeg);
        Serial1.print(", align pos: ");
        Serial1.print(pos_mdeg);
        Serial1.print(", offset: ");
        Serial1.println(m_EncoderOffset_mdeg);
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
        return normalizePosition(m_Encoder->getSensorAngleDegrees() + m_EncoderOffset_mdeg);
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

        // Serial1.print(" target_mdeg: ");
        // Serial1.print(m_Target_mdeg);
        // Serial1.print(" pos_mdeg: ");
        // Serial1.print(pos_mdeg);
        // Serial1.print(" pos_edeg: ");
        // Serial1.print(pos_edeg);
        // Serial1.print(" cmd_edeg: ");
        // Serial1.print(command_edeg);
        // Serial1.print(" dist_mdeg: ");
        // Serial1.print(dist_mdeg);
        // Serial1.print(" distanceFactor: ");
        // Serial1.print(distanceFactor);
        // Serial1.print(" scale: ");
        // Serial1.println(scaleFactor);
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
