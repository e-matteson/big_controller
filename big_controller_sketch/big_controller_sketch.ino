#include <SimpleFOC.h>
#include "tspi.h"
#include "MotorDriver.h"
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
MotorDriver driver {&spi, 0};
BLDCMotor motor = BLDCMotor(7); // TODO what's the "pole pairs number"? Any other args I should give?

MagSensor encoder {Wire, 0x78, -55};
TCA9555 expander(0x20);

void setup() {
    Serial.begin(9600);
    delay(3000);
    Serial.println("setup");
    SimpleFOCDebug::enable();

    pinMode(usr_led_g, OUTPUT);
    pinMode(usr_led_r, OUTPUT);
    digitalWrite(usr_led_r, HIGH);

    Wire.begin();
    expander.begin(INPUT);

    // Turn on encoder
    expander.pinMode1(expander_left_x_enc_vcc, OUTPUT);
    expander.write1(expander_left_x_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder.begin()) {
        Serial.println("failed to initialize tmag5723");
    }

    spi.begin();
    driver.init();

    motor.controller = MotionControlType::angle;
    motor.voltage_limit = driver.m_VoltageLimit;

    // Example tunings
    motor.PID_velocity.P = 0.2;
    motor.PID_velocity.I = 20;
    // jerk control using voltage voltage ramp
    // default value is 300 volts per sec  ~ 0.3V per millisecond
    motor.PID_velocity.output_ramp = 1000;

    // velocity low pass filtering
    // default 5ms - try different values to see what is the best.
    // the lower the less filtered
    motor.LPF_velocity.Tf = 0.01;

    // angle P controller
    // default P=20
    motor.P_angle.P = 20;
    //  maximal velocity of the position control
    // default 20
    motor.velocity_limit = 4;

    // link the motor to the sensor
    motor.linkSensor(&encoder);
    // link the motor to the driver
    motor.linkDriver(&driver);

    // initialize motor
    motor.init();
    // align encoder and start FOC
    motor.initFOC();

    Serial.println("setup done");
}

void toggle_led() {
    static bool state = false;
    state ^= true;
    digitalWrite(usr_led_g, state);
}

void loop() {
    uint32_t timestamp = millis();
    bool state = false;
    while (1) {
        uint32_t now = millis();
        if (timestamp + 1000 < now) {
            timestamp = now;
            state ^= true;
            digitalWrite(usr_led_g, state);

            // encoder.update();
            // Serial.print("angle=");
            // Serial.println(encoder.getAngle());
            if (state) {
                Serial.println("MOVE TO 70");
                motor.move(70);
            }  else {
                Serial.println("MOVE TO 110");
                motor.move(110);
            }
            for (int i = 0; i < driver.m_History.size(); i++) {
                Serial.print(driver.m_History[i]);
                if ((i+1) % 10 == 0) {
                    Serial.println("");
                } else {
                    Serial.print(" ");
                }
            }
            driver.m_History = {0};
            Serial.println("-------");
        }
        // toggle_led();

        // iterative FOC function
        motor.loopFOC();
    }
}

// void demo() {

//     encoder.update();
//     Serial.print("angle=");
//     Serial.println(encoder.getAngle());

//     // for (int i = 0; i < 16; i++) {
//     //     Serial.print(expander.read1(i));
//     //     Serial.print(' ');
//     // }
//     // Serial.println();

//     float scale = 0.75;
//     Serial.println("up");
//     for(uint32_t i = 0; i < 360; i++) {
//         driver.setFromSineTable(i, scale);
//         delay(1);
//     }

//     delay(500);

//     Serial.println("down");
//     for(uint32_t i = 359; i > 0; i--) {
//         driver.setFromSineTable(i, scale);
//         delay(1);
//     }

//     delay(500);
// }
