#pragma once

#include <cstdint>
#include <string>

enum class TradeAction {
    Catalog,
    Buy,
    Sell
};

struct TradeCommand {
    TradeAction action;
    std::string product;
    uint16_t quantity = 0;
};
