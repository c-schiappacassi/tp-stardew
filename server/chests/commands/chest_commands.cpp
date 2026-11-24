#include "chest_commands.h"

#include "chests/chest.h"
#include "player/player_manager.h"

bool ChestCommands::transfer(Chest& chest, PlayerManager& players, uint16_t playerId, const chest_action::Deposit& action) const {
    return players.apply(playerId, player_action::TransferItemsTo{chest.inventory, action.slot, action.quantity}).applied;
}

bool ChestCommands::transfer(Chest& chest, PlayerManager& players, uint16_t playerId, const chest_action::Withdraw& action) const {
    return players.apply(playerId, player_action::TransferItemsFrom{chest.inventory, action.slot, action.quantity}).applied;
}

bool ChestCommands::apply(Chest& chest, PlayerManager& players, uint16_t playerId, const ChestAction& action) const {
    if (players.snapshot(playerId).getOpenChestId() != chest.getId())
        return false;
    return std::visit([&](const auto& request) { return transfer(chest, players, playerId, request); }, action);
}
