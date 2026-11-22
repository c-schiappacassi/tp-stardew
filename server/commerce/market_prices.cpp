#include "market_prices.h"

#include <stdexcept>

uint64_t MarketPrices::buyingPrice(uint64_t basePrice) const {
    return hasDiscount() ? basePrice / 2 : basePrice;
}

uint64_t MarketPrices::sellingPrice(uint64_t basePrice) const {
    return hasDiscount() ? basePrice * 2 : basePrice;
}

bool MarketPrices::hasDiscount() const { return discount == MarketDiscount::Active; }

void MarketPrices::scheduleDiscount() { discount = MarketDiscount::Pending; }

void MarketPrices::finishDay() {
    discount = discount == MarketDiscount::Pending ? MarketDiscount::Active : MarketDiscount::None;
}

MarketDiscount MarketPrices::discountState() const { return discount; }

void MarketPrices::restoreDiscount(MarketDiscount saved) {
    switch (saved) {
        case MarketDiscount::None:
        case MarketDiscount::Pending:
        case MarketDiscount::Active:
            discount = saved;
            return;
    }
    throw std::invalid_argument("Invalid saved market discount");
}
