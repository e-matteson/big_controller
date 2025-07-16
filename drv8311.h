#pragma once

#include <Arduino.h>
#include <optional>

#include "tspi.h"
#include "drv8311_registers.h"

using namespace drv8311_registers;

class Drv8311 {
public:
    Drv8311(TSpi* bus, uint8_t id): m_Bus(bus), m_Id(id) {}

    void begin() {
        // Clear the power-on-reset fault (and any others that happen to be active)
        write(flt_clr_register_t{.flt_clr=1});
    }

    template <typename Reg>
    void write(Reg reg) {
        transfer(false, Reg::address(), reg.encode());
    }

    template <typename Reg>
    std::optional<Reg> read() {
        return Reg::decode(transfer(true, Reg::address(), 0).data);
    }

private:
    struct Result {
        // I think the otp_flt bit isn't actually set and will always be low?
        std::optional<dev_sts1_register_t> status;
        uint16_t data;

        void print() {
            Serial.print("Result: status=");
            if (status.has_value()) {
              Serial.print(status->encode());
            } else {
              Serial.print("(empty)");
            }
            Serial.print(" data=");
            Serial.println(data);
        }
    };

    Result transfer(bool is_read, uint16_t addr, uint16_t tx_data) {
        auto rx_data = m_Bus->transfer(is_read, m_Id, addr, tx_data);

        return Result{
            .status = dev_sts1_register_t::decode(rx_data[0]),
            .data = static_cast<uint16_t>((rx_data[1] << 8) | rx_data[2]),
       };
    }

    TSpi* m_Bus;
    uint8_t m_Id;
};
