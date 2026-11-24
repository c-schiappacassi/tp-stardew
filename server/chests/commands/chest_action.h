#pragma once

#include <cstdint>
#include <variant>

namespace chest_action {
struct Deposit {
    uint8_t slot;
    uint16_t quantity;
};
struct Withdraw {
    uint8_t slot;
    uint16_t quantity;
};
}

using ChestAction = std::variant<chest_action::Deposit, chest_action::Withdraw>;
