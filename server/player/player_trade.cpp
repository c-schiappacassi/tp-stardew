#include "player.h"

#include <limits>
#include <utility>

uint32_t Player::getGold() const { return gold; }

bool Player::buyItem(const Stack& stack, uint32_t price) {
    if (!canAct() || gold < price) return false;
    Inventory prepared = inventory;
    if (!prepared.add(stack)) return false;
    inventory = std::move(prepared);
    gold -= price;
    return true;
}

bool Player::sellItems(const std::vector<std::pair<uint8_t, uint16_t>>& quantities,uint32_t price) {
    if (price > std::numeric_limits<uint32_t>::max() - gold) return false;
    if (!removeItems(quantities)) return false;
    gold += price;
    return true;
}
