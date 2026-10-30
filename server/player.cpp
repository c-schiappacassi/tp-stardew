#include "player.h"

#include <algorithm>
#include <stdexcept>

Player::Player(
    const PlayerData& initialData,
    uint8_t inventoryCapacity,
    uint16_t maxHealth,
    uint16_t maxEnergy,
    uint32_t initialGold):
        pos(initialData.pos),
        state(initialData.state),
        dir(initialData.dir),
        zone(initialData.zone),
        dungeonLevel(initialData.dungeonLevel),
        hairModel(initialData.hairModel),
        shirtModel(initialData.shirtModel),
        pantsModel(initialData.pantsModel),
        hairColor(initialData.hairColor),
        pantsColor(initialData.pantsColor),
        inventory(inventoryCapacity),
        gold(initialGold),
        health(maxHealth),
        maxHealth(maxHealth),
        energy(maxEnergy),
        maxEnergy(maxEnergy) {
    if (maxHealth == 0 || maxEnergy == 0) {
        throw std::invalid_argument(
            "Vida y energia maximas deben ser mayores a cero");
    }
}

void Player::setMovement(Direction direction) {
    if (state == PlayerState::Unconscious || state == PlayerState::Sleeping || state == PlayerState::UsingTool) {
        return;
    }

    switch (direction) {
        case Direction::None:
            state = PlayerState::Idle;
            return;
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            dir = direction;
            state = PlayerState::Walking;
            return;

        default:
            return;
    }
}

void Player::advance(int step) {
    if (state != PlayerState::Walking || step <= 0) {
        return;
    }

    int x = pos.getX();
    int y = pos.getY();

    switch (dir) {
        case Direction::Up: y -= step; break;
        case Direction::Down: y += step; break;
        case Direction::Left: x -= step; break;
        case Direction::Right: x += step; break;
        default: return;
    }

    pos = Position(std::max(0, x), std::max(0, y));
}

bool Player::selectSlot(uint8_t slot) {
    if (slot >= inventory.getCapacity()) {
        return false;
    }

    currentSlot = slot;
    return true;
}

bool Player::addItem(const Stack& stack) {
    return inventory.add(stack);
}

bool Player::removeItem(uint8_t slot, uint16_t quantity) {
    return inventory.remove(slot, quantity);
}

const Stack* Player::getSelectedItem() const {
    return inventory.getSlot(currentSlot);
}

PlayerData Player::getPublicData() const {
    PlayerData data;
    data.pos = pos;
    data.state = state;
    data.dir = dir;
    data.zone = zone;
    data.dungeonLevel = dungeonLevel;
    data.hairModel = hairModel;
    data.shirtModel = shirtModel;
    data.pantsModel = pantsModel;
    data.hairColor = hairColor;
    data.pantsColor = pantsColor;
    return data;
}

PlayerDataLocal Player::getLocalData() const {
    PlayerDataLocal data;
    data.items = inventory.getItems();
    data.current_slot = currentSlot;
    data.gold = gold;
    data.health = health;
    data.maxHealth = maxHealth;
    data.energy = energy;
    data.maxEnergy = maxEnergy;
    return data;
}