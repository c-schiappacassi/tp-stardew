#pragma once

#include "protocol_types.h"

class MarketPrices {
    MarketDiscount discount = MarketDiscount::None;

public:
    uint64_t buyingPrice(uint64_t basePrice) const;
    uint64_t sellingPrice(uint64_t basePrice) const;
    bool hasDiscount() const;
    void scheduleDiscount();
    void finishDay();

    MarketDiscount discountState() const;
    void restoreDiscount(MarketDiscount saved);
};
