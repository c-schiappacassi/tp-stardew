#include "command_router.h"

#include <optional>

#include "player/chat/player_chat.h"
#include "player/player.h"
#include "player/player_manager.h"
#include "player/ports/player_interactions.h"
#include "player/sessions/player_roster.h"
#include "player/time/day_cycle.h"

namespace {
std::optional<Direction> resolveInteractionDirection(const Player& player, Direction requested) {
    switch (requested) {
        case Direction::None:
            return player.getDirection();
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            return requested;
        default:
            return std::nullopt;
    }
}
}  // namespace

CommandRouter::CommandRouter(PlayerManager& manager, PlayerRoster& roster, DayCycle& day,
                             PlayerChat& chat, PlayerInteractions& interactions):
        manager(manager), roster(roster), day(day), chat(chat), interactions(interactions) {}

void CommandRouter::dispatch(PlayerRecord& record, const CommandDTO& command,
                             bool gameCompleted) const {
    if (handleSocialCommand(record, command, day.hasEnded() || gameCompleted))
        return;
    if (gameCompleted)
        return;
    if (handleDayCommand(record, command))
        return;
    if (day.hasEnded())
        return;

    auto& player = record.getPlayer();
    switch (command.type()) {
        case MessageType::Move:
            player.setMovement(static_cast<const MoveDTO&>(command).dir);
            break;
        case MessageType::SelectSlot:
            player.selectSlot(static_cast<const SelectSlotDTO&>(command).slot);
            break;
        case MessageType::Interact:
            interact(record, static_cast<const InteractDTO&>(command).dir);
            break;
        case MessageType::ItemTransfer:
            interactions.depositInChest(manager, record.getId(), static_cast<const ItemTransferDTO&>(command));
            break;
        case MessageType::SelectItem:
            interactions.withdrawFromChest(manager, record.getId(), static_cast<const SelectItemDTO&>(command));
            break;
        case MessageType::CloseChest:
            player.closeChest();
            break;
        default:
            break;
    }
}

bool CommandRouter::handleSocialCommand(PlayerRecord& record, const CommandDTO& command,
                                        bool blocked) const {
    switch (command.type()) {
        case MessageType::Chat:
            chat.handle(roster, record.getId(), static_cast<const ChatDTO&>(command).text, blocked);
            return true;
        case MessageType::Donate:
            chat.donate(record, static_cast<const DonateDTO&>(command), blocked);
            return true;
        default:
            return false;
    }
}

bool CommandRouter::handleDayCommand(PlayerRecord& record, const CommandDTO& command) const {
    switch (command.type()) {
        case MessageType::DayOk:
            day.acknowledge(record.getId(), record.getPlayer(), roster.isMultiplayer());
            return true;
        case MessageType::DayCancel:
            manager.cancelSleep(record.getId());
            return true;
        default:
            return false;
    }
}

void CommandRouter::interact(PlayerRecord& record, Direction requested) const {
    const auto direction = resolveInteractionDirection(record.getPlayer(), requested);
    if (!direction || chat.interact(record, *direction))
        return;
    interactions.interact(manager, record.getId(), *direction);
}

namespace {
PlayerActionResult applyAction(Player& player, const player_action::OpenChest& action) {
    return {player.openChest(action.id), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::CloseChest&) {
    player.closeChest();
    return {true, 0};
}
PlayerActionResult applyAction(Player& player, const player_action::TransferItemsTo& action) {
    return {player.transferItemsTo(action.destination.get(), action.slot, action.quantity), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::TransferItemsFrom& action) {
    return {player.transferItemsFrom(action.source.get(), action.slot, action.quantity), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::FaceDirection& action) {
    player.faceDirection(action.direction);
    return {true, 0};
}
PlayerActionResult applyAction(Player& player, const player_action::FaceTarget& action) {
    player.faceTarget(action.target);
    return {true, 0};
}
PlayerActionResult applyAction(Player& player, const player_action::ChangeLocation& action) {
    return {player.changeLocation(action.zone, action.level, action.position, action.interior), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::RestoreLocation& action) {
    return {player.restoreLocation(action.zone, action.level, action.position, action.interior), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::UseTool& action) {
    return {player.useTool(action.now), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::PerformAction& action) {
    return {player.performAction(action.energyCost, action.interval, action.now), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::Sleep& action) {
    return {player.sleep(action.bed), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::SetVitals& action) {
    player.setVitals(action.health, action.energy);
    return {true, 0};
}
PlayerActionResult applyAction(Player& player, const player_action::RestoreVitals&) {
    player.restoreVitals();
    return {true, 0};
}
PlayerActionResult applyAction(Player& player, const player_action::ApplyCombatBonus&) {
    player.applyCombatBonus();
    return {true, 0};
}
PlayerActionResult applyAction(Player& player, const player_action::ReceiveDamage& action) {
    const auto amount = player.receiveDamage(action.damage, action.now);
    return {amount != 0, amount};
}
PlayerActionResult applyAction(Player& player, const player_action::AddGold& action) {
    const auto amount = player.addGold(action.amount);
    return {amount != 0, amount};
}
PlayerActionResult applyAction(Player& player, const player_action::SpendGold& action) {
    return {player.spendGold(action.amount), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::AddItem& action) {
    return {player.addItem(action.stack), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::AddItemUpTo& action) {
    const auto amount = player.addItemUpTo(action.stack);
    return {amount != 0, amount};
}
PlayerActionResult applyAction(Player& player, const player_action::RemoveItem& action) {
    return {player.removeItem(action.slot, action.quantity), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::ClearSlot& action) {
    return {player.clearSlot(action.slot), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::RemoveItems& action) {
    return {player.removeItems(action.quantities), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::BuyItem& action) {
    return {player.buyItem(action.stack, action.price), 0};
}
PlayerActionResult applyAction(Player& player, const player_action::SellItems& action) {
    return {player.sellItems(action.quantities, action.price), 0};
}
}  // namespace

PlayerActionResult CommandRouter::apply(uint16_t playerId, const PlayerAction& action) const {
    auto* record = roster.find(playerId);
    if (!record)
        return {};
    return std::visit([&](const auto& request) { return applyAction(record->getPlayer(), request); }, action);
}
