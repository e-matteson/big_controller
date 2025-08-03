#include "tspi.h"
#include "motor.h"
#include "mag_sensor.h"
#include <TCA9555.h>

// Pins on the MCU
int dpad_right = 0; // PA22
int drv_sleep_n = 1; // PA23
int button_extra = 2; // PA10
int button_a = 3; // PB10
int button_b = 4; // PB11
int dpad_up = 5; // PA19
int led_clk = 6; // PA2
int bumper_r = 7; // PA18
int led_data = 8; // PA3
int button_start = 9; // PA12
int rumble = 10; // PA13
int button_hat_l = 11; // PA27
int usr_led_g = 12; // PA9
int usr_led_r = 13; // PA8
int button_hat_r = 14; // PA28
int button_y = 15; // PB8
int button_x = 16; // PB9
int button_home = 17; // PA0
int bumper_l = 18; // PA1
int dpad_down = 19; // PA20
int dpad_left = 20; // PA21
int button_back = 21; // PA11
int ana_trigger_l = 22; // PB2
int ana_trigger_r = 23; // PB3

int cs = 31;

// Pins on the expander
int expander_left_x_enc_vcc = 0;
int expander_left_y_enc_vcc = 1;
int expander_right_x_enc_vcc = 2;
int expander_right_y_enc_vcc = 3;
int expander_rumble_l = 4;
int expander_rumble_r = 5;


TSpi spi{&SPI, cs, {5'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz
Motor motor {&spi, 0};

MagSensor sensor {Wire, 0x78};
TCA9555 expander(0x20);

void setup() {
    Serial.begin(9600);
    Serial.println("setup");
    pinMode(usr_led_g, OUTPUT);
    pinMode(usr_led_r, OUTPUT);
    digitalWrite(usr_led_r, HIGH);
    delay(3000);
    Serial.println("delayed");

    spi.begin();
    motor.init();

    Wire.begin();
    Serial.println("begin expander");
    expander.begin(INPUT);
    Serial.print("is expander connected: ");
    Serial.println(expander.isConnected());

    // Turn on encoder
    Serial.println("begin expander");
    expander.pinMode1(expander_left_x_enc_vcc, OUTPUT);
    expander.write1(expander_left_x_enc_vcc, HIGH);

    expander.pinMode1(expander_right_x_enc_vcc, OUTPUT);
    expander.write1(expander_right_x_enc_vcc, LOW);
    delay(1000);

    Serial.println("begin sensor");
    if (!sensor.begin()) {
        Serial.println("failed to initialize tmag5723");
    }

    Serial.println("setup done");
}

void toggle_led() {
    static bool state = false;
    state ^= true;
    digitalWrite(usr_led_g, state);
}

void loop() {
    // Serial.println("loop");
    toggle_led();

    // auto reg = motor.m_Chip.read<drv8311_registers::pwm_ctrl1_register_t>();
    // if (!reg.has_value()) {
    //     Serial.println("read failed");
    // } else {
    //     Serial.println(reg->encode());
    // }
    sensor.update();
    Serial.print("angle=");
    Serial.println(sensor.getAngle());

    for (int i = 0; i < 16; i++) {
        Serial.print(expander.read1(i));
        Serial.print(' ');
    }
    Serial.println();

    float scale = 0.5;
    Serial.println("up");
    for(uint32_t i = 0; i < 360; i++) {
        motor.setFromSineTable(i, scale);
        delay(1);
    }

    delay(500);

    Serial.println("down");
    for(uint32_t i = 359; i > 0; i--) {
        motor.setFromSineTable(i, scale);
        delay(1);
    }

    delay(500);
}

