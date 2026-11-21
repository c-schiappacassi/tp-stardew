#include "cheat_chat.h"

#include <algorithm>
#include <limits>
#include <string_view>
#include <utility>

#include "commerce/shop_products.h"
#include "player/player.h"
#include "world/world_manager.h"

namespace {
std::optional<ItemId> itemNamed(const std::string& name) {
    const std::pair<std::string_view, ItemId> tools[]{
        {"pickaxe", ItemId::Pickaxe}, {"watering_can", ItemId::WateringCan},
        {"shovel", ItemId::Shovel}, {"sword", ItemId::Sword}
    };
    for (const auto& [keyword, item] : tools) {
        if (name == keyword) return item;
    }
    return shop_products::item(name);
}
}

CheatChat::CheatChat(WorldManager& world, std::function<void()> endDay) :
    world(world), endDay(std::move(endDay)) {}

std::string CheatChat::giveItem(Player& player, const CheatCommand& command) const {
    const auto item = itemNamed(command.target);
    if (!item) return "Unknown item. See the item names in docs/cheats.md.";
    if (command.amount == 0 || command.amount > std::numeric_limits<uint16_t>::max()) {
        return "Item amount must be between 1 and 65535.";
    }
    if (!shop_products::salePrice(0, command.quality)) return "Unknown quality.";
    if (!player.addItem({*item, static_cast<uint16_t>(command.amount), command.quality})) {
        return "Not enough inventory space. No items were added. Use /cheat clear_slot N.";
    }
    return "Added " + std::to_string(command.amount) + " " + command.target + ".";
}

std::string CheatChat::warp(const ChatCommandContext& context, const CheatCommand& command) const {
    Zone zone = Zone::InsideHouse;
    std::optional<InteriorLocation> inside;
    if (command.target == "farm") zone = Zone::Farm;
    else if (command.target == "town") zone = Zone::Town;
    else if (command.target == "mine") zone = Zone::Dungeon;
    else if (command.target == "home") {
        if (context.playerId == 0 || context.playerId > 4) return "Your house is not available.";
        inside = {InteriorId::PlayerHouse, static_cast<uint8_t>(context.playerId - 1)};
    } else {
        const std::pair<std::string_view, InteriorId> locations[]{
            {"center", InteriorId::ComunityCenter}, {"pierre", InteriorId::PierresShop},
            {"marnie", InteriorId::MarniesShop}, {"robin", InteriorId::RobinsShop},
            {"clint", InteriorId::ClintsShop}
        };
        for (const auto& [name, interior] : locations) {
            if (command.target == name) inside = {interior, 0};
        }
        if (!inside) return "Unknown destination. Use farm, town, home, center, pierre, marnie, robin, clint or mine.";
    }
    if (zone == Zone::Dungeon && (command.amount == 0 || command.amount > DUNGEON_ROOM_COUNT)) {
        return "Mine level must be between 1 and " + std::to_string(DUNGEON_ROOM_COUNT) + ".";
    }
    if (!world.teleport(context.player, context.playerId, zone,
            zone == Zone::Dungeon ? static_cast<uint8_t>(command.amount) : 0, inside)) {
        return "Cannot teleport: the destination is unavailable or you cannot act. Try /cheat heal.";
    }
    return "Teleported to " + command.target + (zone == Zone::Dungeon ?
        " level " + std::to_string(command.amount) : "") + ".";
}

std::vector<ChatMessageData> CheatChat::execute(
    const ChatCommandContext& context, const CheatCommand& command) {
    const auto reply = [](const std::string& text) {
        return std::vector<ChatMessageData>{{"Cheats", text}};
    };
    if (context.actionsBlocked) return reply("Cheats cannot change the game after the day or game has ended.");
    Player& player = context.player;
    switch (command.action) {
        case CheatAction::SetLife:
        case CheatAction::SetEnergy: {
            if (command.amount > std::numeric_limits<uint16_t>::max()) return reply("Vital amount must be between 0 and 65535.");
            const auto stats = player.getLocalData();
            const bool health = command.action == CheatAction::SetLife;
            player.setVitals(health ? static_cast<uint16_t>(command.amount) : stats.health,
                             health ? stats.energy : static_cast<uint16_t>(command.amount));
            return reply(std::string(health ? "Health" : "Energy") + " set to " + std::to_string(command.amount) + ".");
        }
        case CheatAction::Heal:
            player.restoreVitals();
            return reply("Health and energy restored. You can act again.");
        case CheatAction::AddGold: {
            const auto added = player.addGold(command.amount);
            return reply("Added " + std::to_string(added) + " gold. Total: " + std::to_string(player.getGold()) + ".");
        }
        case CheatAction::RemoveGold: {
            const auto removed = std::min(command.amount, player.getGold());
            player.spendGold(removed);
            return reply("Removed " + std::to_string(removed) + " gold. Total: " + std::to_string(player.getGold()) + ".");
        }
        case CheatAction::NextLevel: {
            const auto level = player.getZone() == Zone::Dungeon ? player.getDungeonLevel() + 1 : 1;
            return reply(warp(context, {CheatAction::Warp, static_cast<uint32_t>(level), "mine"}));
        }
        case CheatAction::CareCrops:
            return reply("Crops ready to harvest: " + std::to_string(world.careCrops()) + ".");
        case CheatAction::CareAnimals:
            return reply("Animals ready to produce: " + std::to_string(world.careAnimals()) + ". Collect milk from cows and eggs from the ground.");
        case CheatAction::GiveItem:
            return reply(giveItem(player, command));
        case CheatAction::ClearSlot: {
            const auto stats = player.getLocalData();
            if (command.amount == 0 || command.amount > stats.cap_inventory) return reply("Slot must be between 1 and " + std::to_string(stats.cap_inventory) + ".");
            const auto slot = static_cast<uint8_t>(command.amount - 1);
            const auto found = stats.items.find(slot);
            if (found == stats.items.end()) return reply("The slot is already empty.");
            player.removeItem(slot, found->second.quantity);
            return reply("Cleared slot " + std::to_string(command.amount) + ".");
        }
        case CheatAction::SpawnAnimal: {
            auto species = shop_products::animal(command.target);
            const auto variant = shop_products::animalVariant(command.target);
            if (!species && variant) species = animalSpeciesForType(*variant);
            if (!species) return reply("Unknown animal. Use cow, chicken, brown_cow, white_cow, brown_chicken, white_chicken, blue_chicken or void_chicken.");
            if (command.amount == 0 || command.amount > std::numeric_limits<uint16_t>::max() ||
                !world.buyAnimals(*species, static_cast<uint16_t>(command.amount), variant)) {
                return reply("Cannot add animals: check the amount, farm space and species limit. No animals were added.");
            }
            return reply("Added " + std::to_string(command.amount) + " " + command.target + " to the farm.");
        }
        case CheatAction::Warp:
            return reply(warp(context, command));
        case CheatAction::SpawnEnemy: {
            if (player.getZone() != Zone::Dungeon) return reply("Enter the mine first: /cheat warp mine 1.");
            const std::pair<std::string_view, EnemyType> types[]{
                {"skeleton", EnemyType::Skeleton}, {"orc", EnemyType::Orc},
                {"vampire_male", EnemyType::VampireMale},
                {"vampire_female", EnemyType::VampireFemale},
                {"vampire_pair", EnemyType::VampireMale}, {"skull", EnemyType::Skull}
            };
            const auto found = std::find_if(std::begin(types), std::end(types),
                [&](const auto& entry) { return entry.first == command.target; });
            if (found == std::end(types)) return reply("Unknown enemy. Use skeleton, orc, vampire_male, vampire_female, vampire_pair or skull.");
            const bool pairs = command.target == "vampire_pair";
            if (command.amount == 0 || command.amount > 256 ||
                !world.spawnEnemies(player, context.playerId, found->second, static_cast<uint16_t>(command.amount),
                                    pairs, context.occupiedPositions)) {
                return reply("Cannot spawn enemies: check your state, free space and the room's enemy limit. No enemies were added.");
            }
            return reply("Spawned " + std::to_string(command.amount * (pairs ? 2 : 1)) +
                         " enemies on this floor.");
        }
        case CheatAction::KillEnemy:
        case CheatAction::KillEnemies: {
            if (player.getZone() != Zone::Dungeon) return reply("Enter the mine first: /cheat warp mine 1.");
            const auto killed = world.killEnemies(player, context.playerId,
                                                  command.action == CheatAction::KillEnemy);
            return reply("Killed " + std::to_string(killed) + " enemies on this floor.");
        }
        case CheatAction::EndDay:
            if (!endDay) return reply("The day cycle is not available.");
            endDay();
            return reply("Time advanced to the end of the day for everyone. Confirm OK to start the next day and save.");
        case CheatAction::CompleteBundle:
        case CheatAction::CompleteAllBundles:
            return reply("The community center is not available.");
    }
    return reply("Unknown cheat. Use /cheats.");
}
