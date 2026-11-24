#pragma once

#include <cstdint>
#include <string>

enum class GiftAction { Offer, Accept, Reject, Cancel };

struct GiftCommand {
    GiftAction action;
    std::string player;
    uint16_t quantity = 1;
};
