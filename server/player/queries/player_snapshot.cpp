#include "player_snapshot.h"

#include "player/player.h"
#include "player/player_config.h"
#include "player/state/player_location.h"

PlayerSnapshot::PlayerSnapshot(const Player& player, const PlayerConfig& config,
                               std::chrono::steady_clock::time_point readyAt):
        data(player.getPublicData()),
        vitals{player.vitals.getHealth(), player.vitals.getMaxHealth(), player.vitals.getEnergy(),player.vitals.getMaxEnergy()},
        gold(player.getGold()),
        inventoryCapacity(player.inventory.getCapacity()),
        selectedSlot(player.getSelectedSlot()),
        selected(player.getSelectedItem() ? std::optional<Stack>{*player.getSelectedItem()} :std::nullopt),
        collision(config.collision),
        hitbox(config.hitbox),
        readyAt(readyAt),
        active(player.canAct()),
        attackable(player.canBeAttacked()),
        resting(player.isResting()),
        sleeping(player.isSleeping()),
        bedAvailable(player.canUseBed()),
        selectedTool(player.getSelectedItem() &&config.tools.contains(player.getSelectedItem()->item_id)),
        swordDamage(player.getSwordDamage()),
        swordRange(player.getSwordRange()),
        miningPower(player.getMiningPower()) {}

const PlayerVitalsData& PlayerSnapshot::getVitals() const { return vitals; }
uint8_t PlayerSnapshot::getInventoryCapacity() const { return inventoryCapacity; }
const Position& PlayerSnapshot::getPosition() const { return data.pos; }
Direction PlayerSnapshot::getDirection() const { return data.dir; }
Zone PlayerSnapshot::getZone() const { return data.currentZone; }
uint8_t PlayerSnapshot::getDungeonLevel() const { return data.dungeonLevel; }
const std::optional<InteriorLocation>& PlayerSnapshot::getInterior() const { return data.interior; }
uint32_t PlayerSnapshot::getGold() const { return gold; }
const Stack* PlayerSnapshot::getSelectedItem() const { return selected ? &*selected : nullptr; }
uint8_t PlayerSnapshot::getSelectedSlot() const { return selectedSlot; }
bool PlayerSnapshot::canAct() const { return active; }
bool PlayerSnapshot::canBeAttacked() const { return attackable; }
bool PlayerSnapshot::isResting() const { return resting; }
bool PlayerSnapshot::isSleeping() const { return sleeping; }
bool PlayerSnapshot::canUseBed() const { return bedAvailable; }
bool PlayerSnapshot::canPerformAction(std::chrono::steady_clock::time_point now) const {
    return active && now >= readyAt;
}
bool PlayerSnapshot::canUseTool(std::chrono::steady_clock::time_point now) const {
    return selectedTool && canPerformAction(now);
}
bool PlayerSnapshot::canUsePickaxe(std::chrono::steady_clock::time_point now) const {
    return canUseTool(now) && getSelectedItem()->item_id == ItemId::Pickaxe;
}
bool PlayerSnapshot::canReach(const Position& target, uint16_t range) const {
    return PlayerLocation{data}.canReach(target, range);
}
CollisionBounds PlayerSnapshot::getCollisionBounds(const Position& position) const {
    return collision.at(position);
}
Position PlayerSnapshot::getCollisionCenter() const { return collision.at(data.pos).center(); }
CollisionBounds PlayerSnapshot::getHitboxBounds() const { return hitbox.at(data.pos); }
Position PlayerSnapshot::getHitboxCenter() const { return getHitboxBounds().center(); }
uint16_t PlayerSnapshot::getSwordDamage() const { return swordDamage; }
uint16_t PlayerSnapshot::getSwordRange() const { return swordRange; }
uint16_t PlayerSnapshot::getMiningPower() const { return miningPower; }
