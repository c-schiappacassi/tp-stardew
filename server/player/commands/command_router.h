#pragma once

#include <optional>

#include "dtos.h"
#include "player_action.h"

class DayCycle;
class Player;
class PlayerChat;
class PlayerManager;
class PlayerInteractions;
class PlayerRecord;
class PlayerRoster;

class CommandRouter {
private:
    PlayerManager& manager;
    PlayerRoster& roster;
    DayCycle& day;
    PlayerChat& chat;
    PlayerInteractions& interactions;

    bool handleSocialCommand(PlayerRecord& record, const CommandDTO& command, bool blocked) const;
    bool handleDayCommand(PlayerRecord& record, const CommandDTO& command) const;
    void interact(PlayerRecord& record, Direction requested) const;

    static std::optional<Direction> resolveInteractionDirection(const Player& player, Direction requested);
    static PlayerActionResult applyAction(Player& player, const player_action::OpenChest& action);
    static PlayerActionResult applyAction(Player& player, const player_action::CloseChest&);
    static PlayerActionResult applyAction(Player& player, const player_action::TransferItemsTo& action);
    static PlayerActionResult applyAction(Player& player, const player_action::TransferItemsFrom& action);
    static PlayerActionResult applyAction(Player& player, const player_action::FaceDirection& action);
    static PlayerActionResult applyAction(Player& player, const player_action::FaceTarget& action);
    static PlayerActionResult applyAction(Player& player, const player_action::ChangeLocation& action);
    static PlayerActionResult applyAction(Player& player, const player_action::RestoreLocation& action);
    static PlayerActionResult applyAction(Player& player, const player_action::UseTool& action);
    static PlayerActionResult applyAction(Player& player, const player_action::PerformAction& action);
    static PlayerActionResult applyAction(Player& player, const player_action::Sleep& action);
    static PlayerActionResult applyAction(Player& player, const player_action::SetVitals& action);
    static PlayerActionResult applyAction(Player& player, const player_action::RestoreVitals&);
    static PlayerActionResult applyAction(Player& player, const player_action::ApplyCombatBonus&);
    static PlayerActionResult applyAction(Player& player, const player_action::ReceiveDamage& action);
    static PlayerActionResult applyAction(Player& player, const player_action::AddGold& action);
    static PlayerActionResult applyAction(Player& player, const player_action::SpendGold& action);
    static PlayerActionResult applyAction(Player& player, const player_action::AddItem& action);
    static PlayerActionResult applyAction(Player& player, const player_action::AddItemUpTo& action);
    static PlayerActionResult applyAction(Player& player, const player_action::RemoveItem& action);
    static PlayerActionResult applyAction(Player& player, const player_action::ClearSlot& action);
    static PlayerActionResult applyAction(Player& player, const player_action::RemoveItems& action);
    static PlayerActionResult applyAction(Player& player, const player_action::BuyItem& action);
    static PlayerActionResult applyAction(Player& player, const player_action::SellItems& action);

public:
    CommandRouter(PlayerManager& manager, PlayerRoster& roster, DayCycle& day, PlayerChat& chat,PlayerInteractions& interactions);
    PlayerActionResult apply(uint16_t playerId, const PlayerAction& action) const;
    void dispatch(PlayerRecord& record, const CommandDTO& command, bool gameCompleted) const;
};
