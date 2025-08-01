#pragma once

#include <Arduino.h>

void assert(bool condition) {
    if (!condition) {
        Serial.println("assertion failed");
        while(1) {}
    }
}
