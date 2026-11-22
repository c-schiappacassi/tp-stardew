#include "player.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {
uint16_t improvedMaximum(uint16_t value) {
    const uint32_t improved = static_cast<uint32_t>(value) * 5 / 4;
    return static_cast<uint16_t>(
            std::min<uint32_t>(std::numeric_limits<uint16_t>::max(), improved));
}
}

Player::Player(
    const PlayerData& initialData, 
    uint8_t inventoryCapacity, 
    uint16_t maxHealth,
    uint16_t maxEnergy, 
    uint32_t initialGold,
     const PlayerConfig& config):
        characterName(initialData.characterName),
        pos(initialData.pos),
        state(initialData.state),
        dir(initialData.dir),
        zone(initialData.currentZone),
        interior(initialData.interior),
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
        throw std::invalid_argument("Maximum health and energy must be greater than zero");
    }
    config.validate();
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
        case Direction::Up:
            y -= step;
            break;
        case Direction::Down:
            y += step;
            break;
        case Direction::Left:
            x -= step;
            break;
        case Direction::Right:
            x += step;
            break;
        default:
            return pos;
    }

    return Position(std::max(0, x), std::max(0, y));
}

void Player::advance(int step) { 
    pos = getNextPosition(step);
 }

PlayerData Player::getPublicData() const {
    PlayerData data;
    data.characterName = characterName;
    data.pos = pos;
    data.state = state;
    data.dir = dir;
    data.currentZone = zone;
    data.interior = interior;
    const Stack* item = getSelectedItem();
    if (item != nullptr) {
        data.heldItem = item->item_id;
    }
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
    data.cap_inventory = inventory.getCapacity();
    data.current_slot = currentSlot;
    data.gold = gold;
    data.health = health;
    data.maxHealth = maxHealth;
    data.energy = energy;
    data.maxEnergy = maxEnergy;
    return data;
}

bool Player::canUseTool(std::chrono::steady_clock::time_point now) const {
    if (!canPerformAction(now)) {
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

    const ToolConfig& tool = config.tools.at(getSelectedItem()->item_id);
    const Direction toolDirection = dir;
    if (!performAction(tool.energyCost, std::chrono::milliseconds{tool.intervalMs}, now)) {
        return false;
    }
    if (state != PlayerState::Unconscious) {
        dir = toolDirection;
        state = PlayerState::UsingTool;
    }
    return true;
}

bool Player::canPerformAction(std::chrono::steady_clock::time_point now) const {
    return canAct() && now >= nextActionTime;
}

bool Player::performAction(uint16_t energyCost, std::chrono::milliseconds interval,
                           std::chrono::steady_clock::time_point now) {
    if (!canPerformAction(now) || interval.count() < 0) {
        return false;
    }

    if (energyCost >= energy) {
        energy = 0;
    } else {
        energy -= energyCost;
    }

    nextActionTime = now + interval;
    if (energy == 0) {
        passOut();
    } else {
        setMovement(movementDirection);
    }

    return true;
}

void Player::updateToolState(std::chrono::steady_clock::time_point now) {
    if (state != PlayerState::UsingTool) {
        return;
    }

    if (now >= nextActionTime) {
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

CollisionBounds Player::getCollisionBounds(const Position& position) const {
    return config.collision.at(position);
}

Position Player::getCollisionCenter() const {
     return getCollisionBounds(pos).center(); 
    }

bool Player::canAct() const {
    return(state == PlayerState::Idle ||state == PlayerState::Walking) &&health > 0 &&energy > 0;
}

bool Player::canBeAttacked() const {
    return health > 0 && energy > 0 && state != PlayerState::Unconscious &&state != PlayerState::Sleeping;
}

bool Player::isResting() const {
    return state == PlayerState::Sleeping || state == PlayerState::Unconscious;
}

bool Player::isSleeping() const { return state == PlayerState::Sleeping; }

bool Player::canUseBed() const { return bedReady && canAct(); }

void Player::leaveBedArea() { bedReady = true; }

const std::optional<InteriorLocation>& Player::getInterior() const { return interior; }

uint16_t Player::receiveDamage(uint16_t damage, std::chrono::steady_clock::time_point now) {
    if (!canBeAttacked() || damage == 0 || now < nextDamageTime) {
        return 0;
    }
    const uint16_t applied = std::min(health, damage);
    health -= applied;
    nextDamageTime = now + std::chrono::milliseconds{config.damageGraceMs};
    if (health == 0) {
        passOut();
    }
    return applied;
}

uint16_t Player::getSwordDamage() const {
    if (static_cast<uint32_t>(health) * 5 <= maxHealth && config.swordDamage > 0) {
        return std::max<uint16_t>(1, config.swordDamage / 2);
    }
    return config.swordDamage;
}

uint16_t Player::getSwordRange() const { return config.swordRange; }

uint32_t Player::addGold(uint32_t amount) {
    const auto received = std::min(amount, std::numeric_limits<uint32_t>::max() - gold);
    gold += received;
    return received;
}

bool Player::spendGold(uint32_t amount) {
    if (gold < amount) return false;
    gold -= amount;
    return true;
}

void Player::applyCombatBonus() {
    if (combatBonus) {
        return;
    }
    maxHealth = improvedMaximum(maxHealth);
    maxEnergy = improvedMaximum(maxEnergy);
    combatBonus = true;
}

void Player::passOut() {
    if (state == PlayerState::Unconscious || state == PlayerState::Sleeping) {
        return;
    }
    gold -= static_cast<uint32_t>(static_cast<uint64_t>(gold) * config.passoutGoldPercent / 100);
    state = PlayerState::Unconscious;
    movementDirection = Direction::None;
}

bool Player::sleep() {
    if (!canAct()) {
        return false;
    }
    state = PlayerState::Sleeping;
    movementDirection = Direction::None;
    return true;
}

bool Player::cancelSleep() {
    if (!isSleeping()) return false;
    state = PlayerState::Idle;
    movementDirection = Direction::None;
    bedReady = false;
    return true;
}

void Player::wakeUp() {
    const bool rested = state == PlayerState::Sleeping;
    health = rested ? maxHealth : std::max<uint16_t>(1, maxHealth / 2);
    energy = rested ? maxEnergy : std::max<uint16_t>(1, maxEnergy / 2);
    state = PlayerState::Idle;
    bedReady = false;
    movementDirection = Direction::None;
    nextActionTime = {};
    nextDamageTime = {};
}

bool Player::changeLocation(Zone destination,uint8_t level,const Position& target,
                            std::optional<InteriorLocation> inside) {
    if (!canAct() || !restoreLocation(destination, level, target, inside)) {
        return false;
    }
    state = PlayerState::Idle;
    return true;
}

bool Player::restoreLocation(Zone destination, uint8_t level, const Position& target,
                             std::optional<InteriorLocation> inside) {
    if (target.getX() < 0 || target.getY() < 0) {
        return false;
    }

    if ((destination == Zone::InsideHouse) != inside.has_value() ||
        (inside && inside->interior_id > InteriorId::ComunityCenter)) {
        return false;
    }
    if (destination == Zone::Dungeon) {
        if (level == 0) {
            return false;
        }
    } else if (destination == Zone::Farm || destination == Zone::Town ||
               destination == Zone::InsideHouse) {
        if (level != 0) {
            return false;
        }

    } else {
        return false;
    }

    pos = target;
    zone = destination;
    interior = inside;
    dungeonLevel = level;
    movementDirection = Direction::None;
    if (state == PlayerState::Walking) state = PlayerState::Idle;

    return true;
}

void Player::faceTarget(const Position& target) {
    if (!canAct()) {
        return;
    }

    const double dx =static_cast<double>(target.getX()) - pos.getX();

    const double dy =static_cast<double>(target.getY()) - pos.getY();

    if (dx == 0 && dy == 0) {
        return;
    }

    if (std::abs(dx) >= std::abs(dy)) {
        faceDirection(dx > 0 ? Direction::Right : Direction::Left);
    } else {
        faceDirection(dy > 0 ? Direction::Down : Direction::Up);
    }
}

void Player::faceDirection(Direction direction) {
    if (!canAct()) {
        return;
    }
    switch (direction) {
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            dir = direction;
            break;
        default:
            break;
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

void Player::restoreData(const PlayerDataLocal& saved) {
    const auto capacity = saved.cap_inventory == 0 ? inventory.getCapacity() : saved.cap_inventory;
    if (saved.current_slot >= capacity || saved.maxHealth == 0 ||
        saved.maxEnergy == 0 || saved.health > saved.maxHealth || saved.energy > saved.maxEnergy) {
        throw std::invalid_argument("Invalid saved player stats");
    }
    Inventory restoredInventory(capacity);
    restoredInventory.restoreSlots(saved.items);
    inventory = std::move(restoredInventory);
    characterName = saved.characterName;
    pos = saved.pos;
    state = saved.state;
    bedReady = false;
    dir = saved.dir;
    movementDirection = Direction::None;
    zone = saved.currentZone;
    interior = saved.interior;
    hairModel = saved.hairModel;
    shirtModel = saved.shirtModel;
    pantsModel = saved.pantsModel;
    hairColor = saved.hairColor;
    pantsColor = saved.pantsColor;
    currentSlot = saved.current_slot;
    gold = saved.gold;
    health = saved.health;
    maxHealth = saved.maxHealth;
    energy = saved.energy;
    maxEnergy = saved.maxEnergy;
}
