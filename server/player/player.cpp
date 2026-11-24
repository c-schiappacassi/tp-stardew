#include "player.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

Player::Player(const PlayerData& initialData, uint8_t inventoryCapacity, uint16_t maxHealth,uint16_t maxEnergy, uint32_t initialGold, const PlayerConfig& config):
        characterName(initialData.characterName),
        location(initialData),
        activity(initialData),
        hairModel(initialData.hairModel),
        shirtModel(initialData.shirtModel),
        pantsModel(initialData.pantsModel),
        hairColor(initialData.hairColor),
        pantsColor(initialData.pantsColor),
        inventory(inventoryCapacity),
        gold(initialGold),
        vitals(maxHealth, maxEnergy),
        config(config) {
    config.validate();
}

void Player::setMovement(Direction direction) { activity.setMovement(direction); }

void Player::advance(int step) {
    if (activity.getState() == PlayerState::Walking)
        location.advance(activity.getDirection(), step);
}

Position Player::getNextPosition(int step) const {
    if (activity.getState() != PlayerState::Walking)
        return location.getPosition();
    return location.nextPosition(activity.getDirection(), step);
}

void Player::faceTarget(const Position& target) {
    if (!canAct())
        return;
    if (const auto direction = location.directionTo(target))
        faceDirection(*direction);
}

void Player::faceDirection(Direction direction) {
    if (canAct())
        activity.faceDirection(direction);
}

int Player::getMovementStep(int normalStep) const { return vitals.movementStep(normalStep); }

Direction Player::getDirection() const { return activity.getDirection(); }

Zone Player::getZone() const { return location.getZone(); }

uint8_t Player::getDungeonLevel() const { return location.getDungeonLevel(); }

const Position& Player::getPosition() const { return location.getPosition(); }

const std::optional<InteriorLocation>& Player::getInterior() const {
    return location.getInterior();
}

bool Player::changeLocation(Zone destination, uint8_t level, const Position& target,std::optional<InteriorLocation> inside) {
    return canAct() && restoreLocation(destination, level, target, inside);
}

bool Player::restoreLocation(Zone destination, uint8_t level, const Position& target,std::optional<InteriorLocation> inside) {
    if (!location.relocate(destination, level, target, inside))
        return false;
    activity.stopMovementAfterRelocation();
    return true;
}

bool Player::canReach(const Position& target, uint16_t range) const {
    return location.canReach(target, range);
}

CollisionBounds Player::getCollisionBounds(const Position& position) const {
    return config.collision.at(position);
}

Position Player::getCollisionCenter() const { return getCollisionBounds(getPosition()).center(); }

CollisionBounds Player::getHitboxBounds() const { return config.hitbox.at(getPosition()); }

Position Player::getHitboxCenter() const { return getHitboxBounds().center(); }

bool Player::canAct() const { return activity.canAct() && vitals.hasResources(); }

bool Player::canBeAttacked() const { return vitals.hasResources() && !activity.isResting(); }

bool Player::isResting() const { return activity.isResting(); }

bool Player::isSleeping() const { return activity.isSleeping(); }

bool Player::canUseBed() const { return activity.isBedReady() && canAct(); }

void Player::leaveBedArea() { activity.leaveBedArea(); }

void Player::passOut() {
    if (activity.passOut())
        gold -= static_cast<uint32_t>(static_cast<uint64_t>(gold) * config.passoutGoldPercent /
                                      100);
}

bool Player::sleep(std::optional<Position> bed) {
    if (!canAct() || (bed && !location.relocate(getZone(), getDungeonLevel(), *bed, getInterior())))
        return false;
    activity.sleep();
    return true;
}

bool Player::cancelSleep() { return activity.cancelSleep(); }

void Player::wakeUp() {
    vitals.recover(isSleeping());
    activity.wakeUp();
    vitals.clearDamageGrace();
}

void Player::setVitals(uint16_t newHealth, uint16_t newEnergy) {
    vitals.set(newHealth, newEnergy);
    if (!vitals.hasResources()) {
        passOut();
    } else if (activity.getState() == PlayerState::Unconscious) {
        resetActivity();
    }
}

void Player::restoreVitals() {
    vitals.recover(true);
    resetActivity();
}

bool Player::canPerformAction(std::chrono::steady_clock::time_point now) const {
    return canAct() && activity.cooldownEnded(now);
}

bool Player::performAction(uint16_t energyCost, std::chrono::milliseconds interval,std::chrono::steady_clock::time_point now) {
    if (!canPerformAction(now) || interval.count() < 0)
        return false;
    vitals.consumeEnergy(energyCost);
    activity.startCooldown(now, interval);
    if (!vitals.hasResources()) {
        passOut();
    } else {
        activity.resumeMovement();
    }
    return true;
}

bool Player::canUsePickaxe(std::chrono::steady_clock::time_point now) const {
    if (!canUseTool(now)) {
        return false;
    }

    return getSelectedItem()->item_id == ItemId::Pickaxe;
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

bool Player::useTool(std::chrono::steady_clock::time_point now) {
    if (!canUseTool(now)) {
        return false;
    }

    const ItemId item = getSelectedItem()->item_id;
    const ToolConfig& tool = config.tools.at(item);
    const Direction toolDirection = activity.getDirection();
    if (!performAction(tool.energyCost, std::chrono::milliseconds{tool.intervalMs}, now)) {
        return false;
    }
    activity.startTool(toolDirection);
    return true;
}

void Player::updateToolState(std::chrono::steady_clock::time_point now) {
    activity.updateToolState(now);
}

uint16_t Player::receiveDamage(uint16_t damage, std::chrono::steady_clock::time_point now) {
    if (!canBeAttacked())
        return 0;
    const auto applied =
            vitals.receiveDamage(damage, now, std::chrono::milliseconds{config.damageGraceMs});
    if (vitals.getHealth() == 0)
        passOut();
    return applied;
}

uint16_t Player::getSwordDamage() const { return vitals.swordDamage(config.swordDamage); }

uint16_t Player::getMiningPower() const { return vitals.hasLowHealth() ? 1 : 2; }

uint16_t Player::getSwordRange() const { return config.swordRange; }

void Player::applyCombatBonus() { vitals.applyCombatBonus(); }

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

bool Player::addItem(const Stack& stack) { return inventory.add(stack); }

uint16_t Player::addItemUpTo(const Stack& stack) { return inventory.addUpTo(stack); }

bool Player::removeItem(uint8_t slot, uint16_t quantity) {
    return inventory.remove(slot, quantity);
}

bool Player::removeItems(const std::vector<std::pair<uint8_t, uint16_t>>& quantities) {
    if (!canAct() || quantities.empty())
        return false;
    Inventory prepared = inventory;
    for (const auto& [slot, quantity]: quantities) {
        if (!prepared.remove(slot, quantity))
            return false;
    }
    inventory = std::move(prepared);
    return true;
}

const std::map<uint8_t, Stack>& Player::getInventoryItems() const { return inventory.getItems(); }

const Stack* Player::getSelectedItem() const { return inventory.getSlot(currentSlot); }

uint8_t Player::getSelectedSlot() const { return currentSlot; }

bool Player::transferItemsTo(Inventory& destination, uint8_t slot, uint16_t quantity) {
    return canAct() && inventory.transferTo(destination, slot, quantity);
}

bool Player::transferItemsTo(Player& destination, uint8_t slot, uint16_t quantity) {
    return canAct() && destination.canAct() && inventory.transferTo(destination.inventory, slot, quantity);
}

bool Player::transferItemsFrom(Inventory& source, uint8_t slot, uint16_t quantity) {
    return canAct() && source.transferTo(inventory, slot, quantity);
}

bool Player::openChest(uint32_t id) {
    const bool validZone = getZone() == Zone::Farm || getZone() == Zone::Dungeon ||
        (getZone() == Zone::InsideHouse && getInterior() && getInterior()->interior_id == InteriorId::PlayerHouse);
    if (id == 0 || !validZone || !canAct())
        return false;
    activity.openChest(id);
    return true;
}

void Player::closeChest() { activity.closeChest(); }

uint32_t Player::addGold(uint32_t amount) {
    const auto received = std::min(amount, std::numeric_limits<uint32_t>::max() - gold);
    gold += received;
    return received;
}

uint32_t Player::getGold() const { return gold; }

bool Player::spendGold(uint32_t amount) {
    if (gold < amount)
        return false;
    gold -= amount;
    return true;
}

bool Player::buyItem(const Stack& stack, uint32_t price) {
    if (!canAct() || gold < price)
        return false;
    Inventory prepared = inventory;
    if (!prepared.add(stack))
        return false;
    inventory = std::move(prepared);
    gold -= price;
    return true;
}

bool Player::sellItems(const std::vector<std::pair<uint8_t, uint16_t>>& quantities,
                       uint32_t price) {
    if (price > std::numeric_limits<uint32_t>::max() - gold)
        return false;
    if (!removeItems(quantities))
        return false;
    gold += price;
    return true;
}

PlayerData Player::getPublicData() const {
    PlayerData data;
    data.characterName = characterName;
    data.pos = location.getPosition();
    data.state = activity.getState();
    data.dir = activity.getDirection();
    data.currentZone = location.getZone();
    data.dungeonLevel = location.getDungeonLevel();
    data.interior = location.getInterior();
    const Stack* item = getSelectedItem();
    if (item != nullptr)
        data.heldItem = item->item_id;
    data.hairModel = hairModel;
    data.shirtModel = shirtModel;
    data.pantsModel = pantsModel;
    data.hairColor = hairColor;
    data.pantsColor = pantsColor;
    data.health = vitals.getHealth();
    data.maxHealth = vitals.getMaxHealth();
    return data;
}

PlayerDataLocal Player::getLocalData() const {
    PlayerDataLocal data(getPublicData());
    data.items = inventory.getItems();
    data.cap_inventory = inventory.getCapacity();
    data.current_slot = currentSlot;
    data.gold = gold;
    data.energy = vitals.getEnergy();
    data.maxEnergy = vitals.getMaxEnergy();
    return data;
}

void Player::restoreData(const PlayerDataLocal& saved) {
    const auto capacity = saved.cap_inventory == 0 ? inventory.getCapacity() : saved.cap_inventory;
    if (saved.current_slot >= capacity)
        throw std::invalid_argument("Invalid saved player stats");

    auto restoredVitals = vitals;
    restoredVitals.restoreData(saved);
    Inventory restoredInventory(capacity);
    restoredInventory.restoreSlots(saved.items);

    inventory = std::move(restoredInventory);
    characterName = saved.characterName;
    location = PlayerLocation{saved};
    activity.restoreData(saved);
    hairModel = saved.hairModel;
    shirtModel = saved.shirtModel;
    pantsModel = saved.pantsModel;
    hairColor = saved.hairColor;
    pantsColor = saved.pantsColor;
    currentSlot = saved.current_slot;
    gold = saved.gold;
    vitals = restoredVitals;
}

void Player::resetActivity() {
    activity.reset();
    vitals.clearDamageGrace();
}

PlayerSnapshot Player::snapshot() const {
    return PlayerSnapshot{*this, config, activity.actionReadyAt()};
}
