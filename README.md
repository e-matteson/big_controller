# Big Controller

## Arduino IDE Setup

I'm using Arduino IDE 2.3.8.

1. Install the custom Big Controller board>

Go to `Preferences > Additional boards manager urls`.
Add `https://raw.githubusercontent.com/e-matteson/big_controller/refs/heads/master/custom_arduino_board/package_e-matteson_bigcontroller_index.json`.
If there are multiple URLs they should be separated with commas.

Then you can select the "Big Controller" board for the sketch, and it will prompt you to download the board files.


2. Install "Arduino SAMD Boards (32-bits ARM Cortex-M0+)" through the Boards Manager, or it won't compile. (I'm using 1.8.14)

3. Install "Simple FOC" library. (I'm using 2.4.0)

4. Install "SparkFun TMAG5273 Arduino Library". (I'm using 2.0.0, the older version was broken.)

5. Install "TCA9555" library. (I'm using 0.4.4)

## Connections

Connect a USB cable to power the MCU.
Connect the barrel jack to 12V to power the motors.
Connect an stlink to the pins labeled G (ground), D (data), and C (clock).
Connect an FTDI cable to get logs on Serial1.

Note: Don't leave 12V connected if nothing else is, it might backpower the MCU.
## Flashing

Uploading through the Arduino IDE does not work. But you can compile with the IDE and then flash the .bin file with gdb or JFlash.


In one terminal:

    cd big_controller
    openocd -f config/openocd_stlink_samd21.cfg

In another terminal, use `gdb-multiarch` or `arm-none-eabi-gdb`:

    cd big_controller
    arm-none-eabi-gdb ~/.cache/arduino/sketches/E9282FBB455DD48548249AC21EA16EB1/big_controller_sketch.ino.elf --batch --command=flash.gdb

or whatever the build directory in `sketches` happens to be named. Consider making a symlink for convenience.

If using `gdb-multiarch`, might need to change `target remote localhost:3333` to `target extended-remote localhost:3333`?


Note: The MCU won't boot if the stlink is plugged into it, even if the stlink isn't powered! Specifically the clock line needs to be disconnected.



