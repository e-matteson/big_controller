#include "TSpi.h"
#include "MotorDriver.h"
#include "Motion.h"
#include "MagSensor.h"
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

TCA9555 expander(0x20);

TSpi spi{&SPI, cs, {5'000'000, MSBFIRST, SPI_MODE1}}; // max baud rate is 10MHz

MotorDriver driver_left_x {&spi, 0};
MagSensor encoder_left_x {Wire, 0x10, 0};
Motion motion_left_x(&driver_left_x, &encoder_left_x, 7);

MotorDriver driver_left_y {&spi, 1};
MagSensor encoder_left_y {Wire, 0x11, 0};
Motion motion_left_y(&driver_left_y, &encoder_left_y, 7);

MotorDriver driver_right_x {&spi, 2};
MagSensor encoder_right_x {Wire, 0x12, 0};
Motion motion_right_x(&driver_right_x, &encoder_right_x, 7);

MotorDriver driver_right_y {&spi, 3};
MagSensor encoder_right_y {Wire, 0x13, 0};
Motion motion_right_y(&driver_right_y, &encoder_right_y, 7);

void setup() {

    pinMode(usr_led_g, OUTPUT);
    pinMode(usr_led_r, OUTPUT);
    digitalWrite(usr_led_r, HIGH);


    Serial1.begin(115200);
    Serial1.println("startup");

    Wire.begin();
    Serial1.println("startup1");
    expander.begin(INPUT);
    Serial1.println("startup2");

    expander.pinMode1(expander_left_x_enc_vcc, OUTPUT);
    expander.pinMode1(expander_left_y_enc_vcc, OUTPUT);
    expander.pinMode1(expander_right_x_enc_vcc, OUTPUT);
    expander.pinMode1(expander_right_y_enc_vcc, OUTPUT);

    Serial1.println("startup3");
    expander.write1(expander_left_x_enc_vcc, LOW);
    expander.write1(expander_left_y_enc_vcc, LOW);
    expander.write1(expander_right_x_enc_vcc, LOW);
    expander.write1(expander_right_y_enc_vcc, LOW);

    Serial1.println("startup4");
    delay(100); // TODO is delay needed for encoder to power on?

    // Turn on encoders one at a time and set their addresses
    // TODO refactor this as a loop
    expander.write1(expander_left_x_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_left_x.begin()) {
        Serial1.println("failed to initialize left x tmag5723");
    }

    Serial1.println("startup5");
    expander.write1(expander_left_y_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_left_y.begin()) {
        Serial1.println("failed to initialize left y tmag5723");
    }

    Serial1.println("startup6");
    expander.write1(expander_right_x_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_right_x.begin()) {
        Serial1.println("failed to initialize right x tmag5723");
    }

    Serial1.println("startup7");
    expander.write1(expander_right_y_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_right_y.begin()) {
        Serial1.println("failed to initialize right y tmag5723");
    }

    Serial1.println("startup8");
    spi.begin();

    driver_left_x.begin();
    driver_left_y.begin();
    driver_right_x.begin();
    driver_right_y.begin();

    motion_left_x.begin();
    motion_left_y.begin();
    motion_right_x.begin();
    motion_right_y.begin();



    motion_left_x.align();
    motion_left_y.align();
    motion_right_x.align();
    motion_right_y.align();

    // 70,21  302,350
    motion_left_x.setTarget(70);
    motion_left_y.setTarget(21);
    motion_right_x.setTarget(302);
    motion_right_y.setTarget(350);

    // motion_left_x.setTarget(0);
    // motion_left_x.align();

    // delay(500);
    // motion_left_x.setTarget(0);

    // delay(500);
    // motion_left_y.setTarget(0);
    // motion_left_y.align();

    // delay(500);
    // motion_left_y.setTarget(0);
    Serial1.println("startup done");
}

void toggle_led() {
    static bool state = false;
    state ^= true;
    digitalWrite(usr_led_g, state);
}

void loop() {
    // spin();
    update_loop();
    // print_encoder();
}

void print_encoder() {
    while (1) {
        delay(500);
        toggle_led();
        Serial1.print("(");
        Serial1.print(encoder_left_x.getSensorAngleDegrees());
        Serial1.print(", ");
        Serial1.print(encoder_left_y.getSensorAngleDegrees());
        Serial1.print(") (");
        Serial1.print(encoder_right_x.getSensorAngleDegrees());
        Serial1.print(", ");
        Serial1.print(encoder_right_y.getSensorAngleDegrees());
        Serial1.println(")");
    }
}

void update_loop() {
    toggle_led();
    uint32_t timestamp = millis();
    while (1) {
        delay(10);
        motion_left_x.update();
        // motion_left_y.update();
        // motion_right_x.update();
        // motion_right_y.update();
        uint32_t now = millis();
        if (now > timestamp + 1000) {
            timestamp = now;
            toggle_led();
        }
    }
}

