#pragma once

#include <SimpleFOC.h>

#include "Drv8311.h"


// This table is for testing, SimpleFOC won't use it
// Max value of lookup table should match the period
static const uint16_t sineLookupTable[360] = {
250, 254, 259, 263, 267, 272, 276, 280, 285, 289, 293, 298, 302, 306, 310, 315, 319, 323, 327, 331,
336, 340, 344, 348, 352, 356, 360, 363, 367, 371, 375, 379, 382, 386, 390, 393, 397, 400, 404, 407,
411, 414, 417, 420, 424, 427, 430, 433, 436, 439, 442, 444, 447, 450, 452, 455, 457, 460, 462, 464,
467, 469, 471, 473, 475, 477, 478, 480, 482, 483, 485, 486, 488, 489, 490, 491, 493, 494, 495, 495,
496, 497, 498, 498, 499, 499, 499, 500, 500, 500, 500, 500, 500, 500, 499, 499, 499, 498, 498, 497,
496, 495, 495, 494, 493, 491, 490, 489, 488, 486, 485, 483, 482, 480, 478, 477, 475, 473, 471, 469,
467, 464, 462, 460, 457, 455, 452, 450, 447, 444, 442, 439, 436, 433, 430, 427, 424, 420, 417, 414,
411, 407, 404, 400, 397, 393, 390, 386, 382, 379, 375, 371, 367, 363, 360, 356, 352, 348, 344, 340,
336, 331, 327, 323, 319, 315, 310, 306, 302, 298, 293, 289, 285, 280, 276, 272, 267, 263, 259, 254,
250, 246, 241, 237, 233, 228, 224, 220, 215, 211, 207, 202, 198, 194, 190, 185, 181, 177, 173, 169,
164, 160, 156, 152, 148, 144, 140, 137, 133, 129, 125, 121, 118, 114, 110, 107, 103, 100, 96, 93,
89, 86, 83, 80, 76, 73, 70, 67, 64, 61, 58, 56, 53, 50, 48, 45, 43, 40, 38, 36,
33, 31, 29, 27, 25, 23, 22, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5,
4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3,
4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 22, 23, 25, 27, 29, 31,
33, 36, 38, 40, 43, 45, 48, 50, 53, 56, 58, 61, 64, 67, 70, 73, 76, 80, 83, 86,
89, 93, 96, 100, 103, 107, 110, 114, 118, 121, 125, 129, 133, 137, 140, 144, 148, 152, 156, 160,
164, 169, 173, 177, 181, 185, 190, 194, 198, 202, 207, 211, 215, 220, 224, 228, 233, 237, 241, 246};


class MotorDriver : public BLDCDriver {
public: 
    MotorDriver(TSpi* bus, uint8_t id)
        : BLDCDriver()
        , m_Chip(bus, id) {
    }

    int init() override {
        m_Chip.begin();

        m_Chip.write(pwmg_period_register_t{m_Period});

        auto pwmg_ctrl = m_Chip.read<pwmg_ctrl_register_t>();
        assert(pwmg_ctrl.has_value());
        pwmg_ctrl->pwm_en = 1;
        // UP_AND_DOWN might cause weirdness when the duty cycle is 0.
        pwmg_ctrl->pwmcntr_mode = pwmcntr_mode_enum::UP;
        m_Chip.write(pwmg_ctrl.value());

        auto drv_ctrl = m_Chip.read<drv_ctrl_register_t>();
        assert(drv_ctrl.has_value());
        drv_ctrl->tdead_ctrl = m_DeadtimeSetting;
        drv_ctrl->dlycmp_en = 0;
        m_Chip.write(drv_ctrl.value());

        // TODO what does the return value mean? should it be 0 or not 0?
        return 0;
    }

    void begin() {
        init();
    }

    /** Enable hardware */
    void enable() override {
        // TODO
    }

    /** Disable hardware */
    void disable() override {
        // TODO
    }

    // In range 0-100
    void setDutyCyclePercents(uint8_t a, uint8_t b, uint8_t c) {
        auto compute = [this](uint8_t percentage) {
            return static_cast<uint16_t>(constrain(percentage * m_Period / 100, 0, m_Period));
        };

        pwmg_a_duty_register_t a_reg{compute(a)};
        pwmg_b_duty_register_t b_reg{compute(b)};
        pwmg_c_duty_register_t c_reg{compute(c)};

        m_Chip.write(a_reg);
        m_Chip.write(b_reg);
        m_Chip.write(c_reg);
    }

    /**
    * Set phase voltages to the hardware
    *
    * @param Ua - phase A voltage
    * @param Ub - phase B voltage
    * @param Uc - phase C voltage
    */
    void setPwm(float Ua, float Ub, float Uc) override {
        auto compute_duty_cycle = [this](float u) {
            float voltage = constrain(u, 0.0f, m_VoltageLimit);
            return static_cast<uint32_t>(voltage * m_Period / m_VoltagePowerSupply);

        };
        pwmg_a_duty_register_t a_reg{compute_duty_cycle(Ua)};
        pwmg_b_duty_register_t b_reg{compute_duty_cycle(Ub)};
        pwmg_c_duty_register_t c_reg{compute_duty_cycle(Uc)};
        // a_reg.print();

        m_Chip.write(a_reg);
        m_Chip.write(b_reg);
        m_Chip.write(c_reg);

        m_History[m_HistoryIndex] = a_reg.encode();
        m_HistoryIndex = (m_HistoryIndex + 1) % m_History.size();
    }

    /**
    * Set phase state, enable/disable
    *
    * @param sc - phase A state : active / disabled ( high impedance )
    * @param sb - phase B state : active / disabled ( high impedance )
    * @param sa - phase C state : active / disabled ( high impedance )
    */
    void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) override {
        // The BLDCDriver6PWM class ignores phase state (at least on the teensy3 implementation)
        // TODO can we ignore it too?
    }

    // pos_edeg must be in [0, 360)
    void commandElectricalAngle(float pos_edeg, float scale_factor) {
        setFromSineTable(static_cast<uint32_t>(round(pos_edeg)), scale_factor);
    }

    void setFromSineTable(uint32_t offset, float scale_factor) {
        uint32_t a_index = offset % 360;
        uint32_t b_index = (offset + 120) % 360;
        uint32_t c_index = (offset + 240) % 360;

        auto compute = [this, scale_factor](uint32_t index) -> uint16_t {
            return static_cast<uint16_t>(round((sineLookupTable[index] + m_HackyOffset) * scale_factor));
        };

        pwmg_a_duty_register_t a_reg{compute(a_index)};
        pwmg_b_duty_register_t b_reg{compute(b_index)};
        pwmg_c_duty_register_t c_reg{compute(c_index)};
        // a_reg.print();
        m_Chip.write(a_reg);
        m_Chip.write(b_reg);
        m_Chip.write(c_reg);

        Serial1.print(", a: ");
        Serial1.print(a_reg.pwm_duty_outa);
        Serial1.print(", b: ");
        Serial1.print(b_reg.pwm_duty_outb);
        Serial1.print(", c: ");
        Serial1.print(c_reg.pwm_duty_outc);
    }

    // TODO private
    Drv8311 m_Chip;


    std::array<uint16_t, 100> m_History = {0};
    uint32_t m_HistoryIndex = 0;

    const float m_VoltageLimit = 6;
    const float m_VoltagePowerSupply= 12;
private:
    // When we command exactly 0% or 1%(?) duty cycle, the driver jumps to 100% duty cycle instead for some reason.
    // So we can add an offset of ~2 to all duty cycles, and the max duty cycle, to avoid that.
    // Or, we can use a deadtime setting of 0us instead, though that might cause the driver to heat up more.
    uint16_t m_HackyOffset = 0;
    tdead_ctrl_enum m_DeadtimeSetting = tdead_ctrl_enum::NO_DEADTIME;

    uint16_t m_Period = 500 + m_HackyOffset;

    // TODO What should the voltage limit and supply voltage actually be?

};
