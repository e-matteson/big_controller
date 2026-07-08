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
MagSensor encoder_left_x {Wire, 0x50, 0};
Motion motion_left_x(&driver_left_x, &encoder_left_x, 7);

MotorDriver driver_left_y {&spi, 1};
MagSensor encoder_left_y {Wire, 0x51, 0};
Motion motion_left_y(&driver_left_y, &encoder_left_y, 7);

MotorDriver driver_right_x {&spi, 2};
MagSensor encoder_right_x {Wire, 0x52, 0};
MagSensor encoder_right_y {Wire, 0x53, 0};

// TODO the right x and y encoders are currently swapped. We should figure out the best way to unswap them.
// Motion motion_right_x(&driver_right_x, &encoder_right_x, 7);
Motion motion_right_x(&driver_right_x, &encoder_right_y, 7);

MotorDriver driver_right_y {&spi, 3};
// Motion motion_right_y(&driver_right_y, &encoder_right_y, 7);
Motion motion_right_y(&driver_right_y, &encoder_right_x, 7);

void setup() {

    pinMode(usr_led_g, OUTPUT);
    pinMode(usr_led_r, OUTPUT);
    digitalWrite(usr_led_r, HIGH);


    Serial1.begin(115200);
    Serial1.println("startup");

    delay(500);
    Wire.begin();
    Serial1.println("startup1");
    expander.begin(INPUT);
    Serial1.println("startup2");

    Serial1.println("startup4");
    delay(100); // TODO is delay needed for encoder to power on?

    // Turn on encoders one at a time and set their addresses
    // TODO refactor this as a loop

    expander.pinMode1(expander_left_x_enc_vcc, OUTPUT);
    expander.write1(expander_left_x_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_left_x.begin()) {
        Serial1.println("failed to initialize left x tmag5723");
    }

    Serial1.println("startup5");
    expander.pinMode1(expander_left_y_enc_vcc, OUTPUT);
    expander.write1(expander_left_y_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_left_y.begin()) {
        Serial1.println("failed to initialize left y tmag5723");
    }

    Serial1.println("startup6");
    expander.pinMode1(expander_right_x_enc_vcc, OUTPUT);
    expander.write1(expander_right_x_enc_vcc, HIGH);
    delay(100); // TODO is delay needed for encoder to power on?
    if (!encoder_right_x.begin()) {
        Serial1.println("failed to initialize right x tmag5723");
    }

    Serial1.println("startup7");
    expander.pinMode1(expander_right_y_enc_vcc, OUTPUT);
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

    { // centering right
      const float default_right_x_command = 90;
      const float default_right_y_command = 90;

      Serial.println("center right x: attempt 1");
      auto right_x_result1 = motion_right_x.findCenter(default_right_x_command);

      Serial.println("center right y: attempt 1");
      auto right_y_result1 = motion_right_y.findCenter(default_right_y_command);

      // delay(1000);
      // Serial.println("center right x: attempt 2");
      // auto right_x_result2 = motion_right_x.findCenter(right_x_result1.value_or(default_right_x_command));

      // Serial.println("center right y: attempt 2");
      // auto right_y_result2 =motion_right_y.findCenter(right_y_result1.value_or(default_right_y_command));

      if (right_x_result1.has_value()) {
          motion_right_x.align(right_x_result1.value());
      } else {
          Serial.println("Failed to center right x axis");
      }
      if (right_y_result1.has_value()) {
          motion_right_y.align(right_y_result1.value());
      } else {
          Serial.println("Failed to center right y axis");
      }
    }
    { // centering left
      const float default_left_x_command = 90;
      const float default_left_y_command = 0;

      Serial.println("center left x: attempt 1");
      auto left_x_result1 = motion_left_x.findCenter(default_left_x_command);

      Serial.println("center left y: attempt 1");
      auto left_y_result1 = motion_left_y.findCenter(default_left_y_command);

      // Serial.println("center left x: attempt 2");
      // auto left_x_result2 = motion_left_x.findCenter(left_x_result1.value_or(default_left_x_command));

      // Serial.println("center left y: attempt 2");
      // auto left_y_result2 =motion_left_y.findCenter(left_y_result1.value_or(default_left_y_command));

      if (left_x_result1.has_value()) {
          motion_left_x.align(left_x_result1.value());
      } else {
          Serial.println("Failed to center left x axis");
      }
      if (left_y_result1.has_value()) {
          motion_left_y.align(left_y_result1.value());
      } else {
          Serial.println("Failed to center left y axis");
      }
    }


    motion_left_x.setTarget(0);
    motion_left_y.setTarget(0);
    motion_right_x.setTarget(0);
    motion_right_y.setTarget(0);

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
    auto lx_mdeg = motion_left_x.getPosition_mdeg();
    auto ly_mdeg = motion_left_y.getPosition_mdeg();
    auto rx_mdeg = motion_right_x.getPosition_mdeg();
    auto ry_mdeg = motion_right_y.getPosition_mdeg();
  
    // Serial1.print("mdeg: ");
    // Serial1.print(lx_mdeg);
    // Serial1.print("  edeg: ");
    // Serial1.println(motion_left_x.mechToElecDeg(lx_mdeg));
    
    Serial1.print("mdeg: (");
    Serial1.print(lx_mdeg);
    Serial1.print(", ");
    Serial1.print(ly_mdeg);
    Serial1.print(") (");
    Serial1.print(rx_mdeg);
    Serial1.print(", ");
    Serial1.print(ry_mdeg);
    Serial1.print(")  edeg: (");
    Serial1.print(motion_left_x.mechToElecDeg(lx_mdeg));
    Serial1.print(", ");
    Serial1.print(motion_left_y.mechToElecDeg(ly_mdeg));
    Serial1.print(") (");
    Serial1.print(motion_right_x.mechToElecDeg(rx_mdeg));
    Serial1.print(", ");
    Serial1.print(motion_right_y.mechToElecDeg(ry_mdeg));
    Serial1.println(")");
}

void update_loop() {
    toggle_led();
    uint32_t timestamp = millis();
    while (1) {
        delay(10);
        motion_left_x.update();
        motion_left_y.update();
        motion_right_x.update();
        motion_right_y.update();
        uint32_t now = millis();
        if (now > timestamp + 500) {
            timestamp = now;
            toggle_led();
            // Serial1.println("blink");
            print_encoder();
        }
    }
}

