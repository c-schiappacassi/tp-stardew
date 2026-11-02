#include "player.h"

#include <algorithm>
#include <stdexcept>
#include <cmath>

Player::Player(
    const PlayerData& initialData,
    uint8_t inventoryCapacity,
    uint16_t maxHealth,
    uint16_t maxEnergy,
    uint32_t initialGold,
    const PlayerConfig& config):
        pos(initialData.pos),
        state(initialData.state),
        dir(initialData.dir),
        zone(initialData.currentZone),
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
        maxEnergy(maxEnergy),
        config(config) {
    if (maxHealth == 0 || maxEnergy == 0) {
        throw std::invalid_argument("Vida y energia maximas deben ser mayores a cero");
    }
    if (state == PlayerState::Walking) {
        movementDirection = dir;
    }
}

void Player::setMovement(Direction direction) {
    switch (direction) {
        case Direction::None:
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            break;
        default:
            return;
    }

    if (state == PlayerState::Unconscious || state == PlayerState::Sleeping) {
        return;
    }

    movementDirection = direction;

    if (state == PlayerState::UsingTool) {
        return;
    }
    if (direction == Direction::None) {
        state = PlayerState::Idle;
        return;
    }

    dir = direction;
    state = PlayerState::Walking;
}

Position Player::getNextPosition(int step) const {
    if (state != PlayerState::Walking || step <= 0) {
        return pos;
    }

    int x = pos.getX();
    int y = pos.getY();

    switch (dir) {
        case Direction::Up: y -= step; break;
        case Direction::Down: y += step; break;
        case Direction::Left: x -= step; break;
        case Direction::Right: x += step; break;
        default: return pos;
    }

    return Position(std::max(0, x), std::max(0, y));
}

void Player::advance(int step) {
    pos = getNextPosition(step);
}

bool Player::selectSlot(uint8_t slot) {
    if (!canAct()) {
        return false;
    }
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
    data.currentZone = zone;    
    data.hairModel = hairModel;
    data.shirtModel = shirtModel;
    data.pantsModel = pantsModel;
    data.hairColor = hairColor;
    data.pantsColor = pantsColor;
    return data;
}

PlayerDataLocal Player::getLocalData() const {
    PlayerDataLocal data(getPublicData());
    data.items = inventory.getItems();
    data.current_slot = currentSlot;
    data.gold = gold;
    data.health = health;
    data.maxHealth = maxHealth;
    data.energy = energy;
    data.maxEnergy = maxEnergy;
    return data;
}

bool Player::canUseTool(std::chrono::steady_clock::time_point now) const {
    if (!canAct()) {
        return false;
    }
    if (now < nextToolUseTime) {
        return false;
    }

    const Stack* item = getSelectedItem();
    if (item == nullptr) {
        return false;
    }

    return config.tools.contains(item->item_id);
}

bool Player::canUsePickaxe(std::chrono::steady_clock::time_point now) const {
    if (!canUseTool(now)) {
        return false;
    }

    return getSelectedItem()->item_id == ItemId::Pickaxe;
}

bool Player::useTool(std::chrono::steady_clock::time_point now) {

    if (!canUseTool(now)) {
        return false;
    }

    const ToolConfig& tool =config.tools.at(getSelectedItem()->item_id);

    if (tool.energyCost >= energy) {
        energy = 0;
    } else {
        energy -= tool.energyCost;
    }

    nextToolUseTime =now + std::chrono::milliseconds(tool.intervalMs);
    if (energy == 0) {
        state = PlayerState::Unconscious;
        movementDirection = Direction::None;
    } else {
        state = PlayerState::UsingTool;
    }

    return true;
}

void Player::updateToolState(
    std::chrono::steady_clock::time_point now) {
    if (state != PlayerState::UsingTool) {
        return;
    }

    if (now >= nextToolUseTime) {
        state = PlayerState::Idle;
        setMovement(movementDirection);
    }
}

bool Player::canReach(const Position& target, uint16_t range) const {
    const double distanceX =static_cast<double>(pos.getX()) - target.getX();
    const double distanceY =static_cast<double>(pos.getY()) - target.getY();

    const double distanceSquared =distanceX * distanceX + distanceY * distanceY;
    const double rangeSquared =static_cast<double>(range) * range;

    return distanceSquared <= rangeSquared;
}

Zone Player::getZone() const {
    return zone;
}

uint8_t Player::getDungeonLevel() const {
    return dungeonLevel;
}

const Position& Player::getPosition() const {
    return pos;
}

bool Player::canAct() const {
    return(state == PlayerState::Idle ||state == PlayerState::Walking) &&health > 0 &&energy > 0;
}

bool Player::changeLocation(Zone destination,uint8_t level,const Position& target) {
    if (!canAct() ||target.getX() < 0 ||target.getY() < 0) {
        return false;
    }

    if (destination == Zone::Dungeon) {
        if (level == 0) {
            return false;
        }
    } else if ( destination == Zone::Farm ||destination == Zone::Town) {
        if (level != 0) {
            return false;
        }

    } else {
        return false;
    }

    pos = target;
    zone = destination;
    dungeonLevel = level;

    return true;
}

void Player::faceTarget(const Position& target) {
    if (!canAct()) {
        return;
    }

    const double dx =
        static_cast<double>(target.getX()) - pos.getX();

    const double dy =
        static_cast<double>(target.getY()) - pos.getY();

    if (dx == 0 && dy == 0) {
        return;
    }

    if (std::abs(dx) >= std::abs(dy)) {
        if (dx > 0) {
            dir = Direction::Right;
        } else {
            dir = Direction::Left;
        }
    } else {
        if (dy > 0) {
            dir = Direction::Down;
        } else {
            dir = Direction::Up;
        }
    }
}

int Player::getMovementStep(int normalStep) const {
    if (normalStep <= 0) {
        return 0;
    }

    if (static_cast<uint32_t>(energy) * 10 <= maxEnergy) {
        return std::max(1, normalStep / 2);
    }

    return normalStep;
}
