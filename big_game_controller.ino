#include "tspi.h"
#include "drv8311.h"

const uint16_t sineLookupTable[360] = {
125, 127, 129, 132, 134, 136, 138, 140,
142, 145, 147, 149, 151, 153, 155, 157,
159, 162, 164, 166, 168, 170, 172, 174,
176, 178, 180, 182, 184, 186, 188, 189,
191, 193, 195, 197, 198, 200, 202, 204,
205, 207, 209, 210, 212, 213, 215, 216,
218, 219, 221, 222, 224, 225, 226, 227,
229, 230, 231, 232, 233, 234, 235, 236,
237, 238, 239, 240, 241, 242, 242, 243,
244, 245, 245, 246, 246, 247, 247, 248,
248, 248, 249, 249, 249, 250, 250, 250,
250, 250, 250, 250, 250, 250, 250, 250,
249, 249, 249, 248, 248, 248, 247, 247,
246, 246, 245, 245, 244, 243, 242, 242,
241, 240, 239, 238, 237, 236, 235, 234,
233, 232, 231, 230, 229, 227, 226, 225,
224, 222, 221, 219, 218, 216, 215, 213,
212, 210, 209, 207, 205, 204, 202, 200,
198, 197, 195, 193, 191, 189, 188, 186,
184, 182, 180, 178, 176, 174, 172, 170,
168, 166, 164, 162, 159, 157, 155, 153,
151, 149, 147, 145, 142, 140, 138, 136,
134, 132, 129, 127, 125, 123, 121, 118,
116, 114, 112, 110, 108, 105, 103, 101,
99, 97, 95, 93, 91, 88, 86, 84,
82, 80, 78, 76, 74, 72, 70, 68,
66, 64, 62, 61, 59, 57, 55, 53,
52, 50, 48, 46, 45, 43, 41, 40,
38, 37, 35, 34, 32, 31, 29, 28,
26, 25, 24, 23, 21, 20, 19, 18,
17, 16, 15, 14, 13, 12, 11, 10,
9, 8, 8, 7, 6, 5, 5, 4,
4, 3, 3, 2, 2, 2, 1, 1,
1, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 1, 1, 1, 2,
2, 2, 3, 3, 4, 4, 5, 5,
6, 7, 8, 8, 9, 10, 11, 12,
13, 14, 15, 16, 17, 18, 19, 20,
21, 23, 24, 25, 26, 28, 29, 31,
32, 34, 35, 37, 38, 40, 41, 43,
45, 46, 48, 50, 52, 53, 55, 57,
59, 61, 62, 64, 66, 68, 70, 72,
74, 76, 78, 80, 82, 84, 86, 88,
91, 93, 95, 97, 99, 101, 103, 105,
108, 110, 112, 114, 116, 118, 121, 123};

TSpi spi{&SPI, 10, {1'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
Drv8311 driver {&spi, 3};

void assert(bool condition) {
    if (!condition) {
        Serial.println("assertion failed");
        while(1) {}
    }
}

void scan_duties(int max_percent) {
    int raw_max = 1000;
    for (int percent = 0; percent <= max_percent; percent += 1) {
        uint16_t duty = percent * raw_max / 100;
        auto a_reg = pwmg_a_duty_register_t{duty};
        // a_reg.print();
        driver.write(a_reg);
        driver.write(pwmg_b_duty_register_t{duty});
        driver.write(pwmg_c_duty_register_t{duty});
        delay(2);
    }
    for (int percent = max_percent; percent > 0; percent -= 1) {
        uint16_t duty = percent * raw_max / 100;
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

    driver.write(pwmg_period_register_t{500});

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


    // driver.write(pwmg_a_duty_register_t{20});
    // driver.write(pwmg_b_duty_register_t{30});
    // driver.write(pwmg_c_duty_register_t{40});
}

uint16_t lookup(uint32_t index) {
    uint16_t offset = 1;
    return (sineLookupTable[index] * 3 / 2) + offset;
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
