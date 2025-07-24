#include "tspi.h"
#include "drv8311.h"
#include <cstdint>

// Max value of lookup table should match the period
const uint16_t PERIOD = 500;

const uint16_t sineLookupTable[360] = {
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

TSpi spi{&SPI, 10, {1'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
Drv8311 driver {&spi, 3};

void assert(bool condition) {
    if (!condition) {
        Serial.println("assertion failed");
        while(1) {}
    }
}

void scan_duties(int max_percent) {
    for (int percent = 0; percent <= max_percent; percent += 1) {
        uint16_t duty = percent * PERIOD / 100;
        auto a_reg = pwmg_a_duty_register_t{duty};
        // a_reg.print();
        driver.write(a_reg);
        driver.write(pwmg_b_duty_register_t{duty});
        driver.write(pwmg_c_duty_register_t{duty});
        delay(2);
    }
    for (int percent = max_percent; percent > 0; percent -= 1) {
        uint16_t duty = percent * PERIOD / 100;
        auto a_reg = pwmg_a_duty_register_t{duty};
        // a_reg.print();
        driver.write(a_reg);
        driver.write(pwmg_b_duty_register_t{duty});
        driver.write(pwmg_c_duty_register_t{duty});
        delay(2);
    }
}

void setup() {
    Serial.begin(9600);
    spi.begin();
    driver.begin();

    driver.write(pwmg_period_register_t{PERIOD});

    auto pwmg_ctrl = driver.read<pwmg_ctrl_register_t>();
    assert(pwmg_ctrl.has_value());
    pwmg_ctrl->pwm_en = 1;
    driver.write(pwmg_ctrl.value());

    auto drv_ctrl = driver.read<drv_ctrl_register_t>();
    assert(drv_ctrl.has_value());
    drv_ctrl->tdead_ctrl = tdead_ctrl_enum::_200NS;
    drv_ctrl->slew_rate = slew_rate_enum::_230_V_PER_US;
    // drv_ctrl->dlycmp_en = 1;
    driver.write(drv_ctrl.value());
}

uint16_t lookup(uint32_t index) {
    uint16_t offset = 0;
    uint16_t scale_numerator = 2;
    uint16_t scale_denominator = 3;
    uint32_t scaled_duty = sineLookupTable[index] * scale_numerator / scale_denominator;
    return  min(scaled_duty + offset, PERIOD);
}

void loop() {
    for(uint32_t i = 0; i < UINT32_MAX; i++) {
        uint32_t a_index = i % 360;
        uint32_t b_index = (i + 120) % 360;
        uint32_t c_index = (i + 240) % 360;

        pwmg_a_duty_register_t a_reg{lookup(a_index)};
        pwmg_b_duty_register_t b_reg{lookup(b_index)};
        pwmg_c_duty_register_t c_reg{lookup(c_index)};

        driver.write(a_reg);
        driver.write(b_reg);
        driver.write(c_reg);
        // a_reg.print();
        // b_reg.print();
        // c_reg.print();
        delay(1);
    }

}
