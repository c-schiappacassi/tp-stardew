#include "cheat_chat.h"

#include "server_constants.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <string_view>
#include <utility>

#include "commerce/shop_products.h"
#include "player/player_manager.h"
#include "world/world_manager.h"

namespace {
constexpr std::pair<std::string_view, ItemId> tools[]{
    {server_constants::PICKAXE, ItemId::Pickaxe},
    {server_constants::WATERING_CAN, ItemId::WateringCan},
    {server_constants::SHOVEL, ItemId::Shovel},
    {server_constants::SWORD, ItemId::Sword}
};

std::optional<ItemId> itemNamed(const std::string& name) {
    for (const auto& [keyword, item]: tools) {
        if (name == keyword)
            return item;
    }
    return shop_products::item(name);
}

std::vector<ChatMessageData> listGiveItems() {
    constexpr std::size_t LINE_LENGTH = 100;
    std::vector<ChatMessageData> messages{{server_constants::CHEATS, server_constants::GIVE_ITEMS_HEADER}};
    std::vector<std::string_view> names;
    for (const auto& tool: tools)
        names.push_back(tool.first);
    const auto products = shop_products::itemNames();
    names.insert(names.end(), products.begin(), products.end());

    std::string line;
    for (const auto name: names) {
        if (!line.empty() && line.size() + std::size(server_constants::ITEM_LIST_SEPARATOR) - 1 + name.size() > LINE_LENGTH) {
            messages.push_back({server_constants::CHEATS, std::move(line)});
            line.clear();
        }
        if (!line.empty())
            line += server_constants::ITEM_LIST_SEPARATOR;
        line += name;
    }
    if (!line.empty())
        messages.push_back({server_constants::CHEATS, std::move(line)});
    messages.push_back({server_constants::CHEATS, server_constants::COMMAND_CHEAT_GIVE_PRODUCT_N_NONE_BASE_GOOD_EXCELLENT_INCREDIBLE_PERFECT});
    return messages;
}
}  // namespace

CheatChat::CheatChat(WorldManager& world, std::function<void()> endDay): world(world), endDay(std::move(endDay)) {}

std::string CheatChat::giveItem(PlayerManager& players, uint16_t playerId, const CheatCommand& command) const {
    const auto item = itemNamed(command.target);
    if (!item)
        return server_constants::UNKNOWN_ITEM_USE_CHEAT_ITEMS;
    if (command.amount == 0 || command.amount > std::numeric_limits<uint16_t>::max()) {
        return server_constants::ITEM_AMOUNT_MUST_BE_BETWEEN_1_AND_65535;
    }
    if (!shop_products::salePrice(0, command.quality))
        return server_constants::UNKNOWN_QUALITY;
    if (!players.apply(playerId, player_action::AddItem{{*item, static_cast<uint16_t>(command.amount), command.quality}}).applied) {
        return server_constants::NOT_ENOUGH_INVENTORY_SPACE_NO_ITEMS_WERE_ADDED_USE_CHEAT_CLEAR_SLOT;
    }
    return server_constants::ADDED_PREFIX + std::to_string(command.amount) + server_constants::SPACE + command.target + server_constants::DOT;
}

std::string CheatChat::warp(const ChatCommandContext& context, const CheatCommand& command) const {
    Zone zone = Zone::InsideHouse;
    std::optional<InteriorLocation> inside;
    if (command.target == server_constants::FARM)
        zone = Zone::Farm;
    else if (command.target == server_constants::TOWN)
        zone = Zone::Town;
    else if (command.target == server_constants::MINE)
        zone = Zone::Dungeon;
    else if (command.target == server_constants::HOME) {
        if (context.playerId == 0 || context.playerId > 4)
            return server_constants::YOUR_HOUSE_IS_NOT_AVAILABLE;
        inside = {InteriorId::PlayerHouse, static_cast<uint8_t>(context.playerId - 1)};
    } else {
        const std::pair<std::string_view, InteriorId> locations[]{{server_constants::CENTER, InteriorId::ComunityCenter}, {server_constants::PIERRE_2, InteriorId::PierresShop}, {server_constants::MARNIE_2, InteriorId::MarniesShop},
            {server_constants::ROBIN_2, InteriorId::RobinsShop}, {server_constants::CLINT_2, InteriorId::ClintsShop}};
        for (const auto& [name, interior]: locations) {
            if (command.target == name)
                inside = {interior, 0};
        }
        if (!inside)
            return server_constants::UNKNOWN_DESTINATION_USE_FARM_TOWN_HOME_CENTER_PIERRE_MARNIE_ROBIN_CLINT_OR;
    }
    if (zone == Zone::Dungeon && (command.amount == 0 || command.amount > DUNGEON_ROOM_COUNT)) {
        return server_constants::MINE_LEVEL_MUST_BE_BETWEEN_1_AND_PREFIX + std::to_string(DUNGEON_ROOM_COUNT) + server_constants::DOT;
    }
    if (!world.teleport(context.players, context.playerId, zone, zone == Zone::Dungeon ? static_cast<uint8_t>(command.amount) : 0, inside)) {
        return server_constants::CANNOT_TELEPORT_THE_DESTINATION_IS_UNAVAILABLE_OR_YOU_CANNOT_ACT_TRY_CHEAT;
    }
    return server_constants::TELEPORTED_TO_PREFIX + command.target + (zone == Zone::Dungeon ? server_constants::LEVEL_SEPARATOR + std::to_string(command.amount) : server_constants::EMPTY) + server_constants::DOT;
}

std::vector<ChatMessageData> CheatChat::execute(const ChatCommandContext& context, const CheatCommand& command) {
    const auto reply = [](const std::string& text) { return std::vector<ChatMessageData>{{server_constants::CHEATS, text}}; };
    if (context.actionsBlocked && command.action != CheatAction::ListItems)
        return reply(server_constants::CHEATS_CANNOT_CHANGE_THE_GAME_AFTER_THE_DAY_OR_GAME_HAS_ENDED);
    auto& players = context.players;
    const auto playerId = context.playerId;
    const auto player = players.snapshot(playerId);
    switch (command.action) {
        case CheatAction::ListItems:
            return listGiveItems();
        case CheatAction::SetLife:
        case CheatAction::SetEnergy: {
            if (command.amount > std::numeric_limits<uint16_t>::max())
                return reply(server_constants::VITAL_AMOUNT_MUST_BE_BETWEEN_0_AND_65535);
            const auto stats = player.getVitals();
            const bool health = command.action == CheatAction::SetLife;
            players.apply(
                playerId, player_action::SetVitals{health ? static_cast<uint16_t>(command.amount) : stats.health, health ? stats.energy : static_cast<uint16_t>(command.amount)});
            return reply(std::string(health ? server_constants::HEALTH : server_constants::ENERGY) + server_constants::SET_TO_SEPARATOR + std::to_string(command.amount) + server_constants::DOT);
        }
        case CheatAction::Heal:
            players.apply(playerId, player_action::RestoreVitals{});
            return reply(server_constants::HEALTH_AND_ENERGY_RESTORED_YOU_CAN_ACT_AGAIN);
        case CheatAction::AddGold: {
            const auto added = players.apply(playerId, player_action::AddGold{command.amount}).amount;
            return reply(server_constants::ADDED_PREFIX + std::to_string(added) + server_constants::GOLD_TOTAL_SEPARATOR + std::to_string(players.snapshot(playerId).getGold()) + server_constants::DOT);
        }
        case CheatAction::RemoveGold: {
            const auto removed = std::min(command.amount, player.getGold());
            players.apply(playerId, player_action::SpendGold{removed});
            return reply(server_constants::REMOVED_PREFIX + std::to_string(removed) + server_constants::GOLD_TOTAL_SEPARATOR + std::to_string(players.snapshot(playerId).getGold()) + server_constants::DOT);
        }
        case CheatAction::NextLevel: {
            const auto level = player.getZone() == Zone::Dungeon ? player.getDungeonLevel() + 1 : 1;
            return reply(warp(context, {CheatAction::Warp, static_cast<uint32_t>(level), server_constants::MINE}));
        }
        case CheatAction::CareCrops:
            return reply(server_constants::CROPS_READY_TO_HARVEST_PREFIX + std::to_string(world.careCrops()) + server_constants::DOT);
        case CheatAction::CareAnimals:
            return reply(server_constants::ANIMALS_READY_TO_PRODUCE_PREFIX + std::to_string(world.careAnimals()) + server_constants::FIELD_COLLECT_MILK_FROM_COWS_AND_EGGS_FROM_THE_GROUND);
        case CheatAction::GiveItem:
            return reply(giveItem(players, playerId, command));
        case CheatAction::ClearSlot: {
            const auto capacity = player.getInventoryCapacity();
            const auto items = players.inventory(playerId);
            if (command.amount == 0 || command.amount > capacity)
                return reply(server_constants::SLOT_MUST_BE_BETWEEN_1_AND_PREFIX + std::to_string(capacity) + server_constants::DOT);
            const auto slot = static_cast<uint8_t>(command.amount - 1);
            const auto found = items.find(slot);
            if (found == items.end())
                return reply(server_constants::THE_SLOT_IS_ALREADY_EMPTY);
            if (!players.apply(playerId, player_action::ClearSlot{slot}).applied)
                return reply(server_constants::TOOLS_CANNOT_BE_DISCARDED_WITH_CLEAR_SLOT);
            return reply(server_constants::CLEARED_SLOT_PREFIX + std::to_string(command.amount) + server_constants::DOT);
        }
        case CheatAction::SpawnAnimal: {
            auto species = shop_products::animal(command.target);
            const auto variant = shop_products::animalVariant(command.target);
            if (!species && variant)
                species = animalSpeciesForType(*variant);
            if (!species)
                return reply(server_constants::UNKNOWN_ANIMAL_USE_COW_CHICKEN_BROWN_COW_WHITE_COW_BROWN_CHICKEN_WHITE);
            if (command.amount == 0 || command.amount > std::numeric_limits<uint16_t>::max() || !world.buyAnimals(*species, static_cast<uint16_t>(command.amount), variant)) {
                return reply(server_constants::CANNOT_ADD_ANIMALS_CHECK_THE_AMOUNT_FARM_SPACE_AND_SPECIES_LIMIT_NO);
            }
            return reply(server_constants::ADDED_PREFIX + std::to_string(command.amount) + server_constants::SPACE + command.target + server_constants::TO_THE_FARM_SUFFIX);
        }
        case CheatAction::Warp:
            return reply(warp(context, command));
        case CheatAction::SpawnEnemy: {
            if (player.getZone() != Zone::Dungeon)
                return reply(server_constants::ENTER_THE_MINE_FIRST_CHEAT_WARP_MINE_1);
            const std::pair<std::string_view, EnemyType> types[]{{server_constants::SKELETON, EnemyType::Skeleton}, {server_constants::ORC, EnemyType::Orc}, {server_constants::VAMPIRE_MALE, EnemyType::VampireMale},
                {server_constants::VAMPIRE_FEMALE, EnemyType::VampireFemale}, {server_constants::VAMPIRE_PAIR, EnemyType::VampireMale}, {server_constants::SKULL, EnemyType::Skull}};
            const auto found = std::find_if(std::begin(types), std::end(types), [&](const auto& entry) { return entry.first == command.target; });
            if (found == std::end(types))
                return reply(server_constants::UNKNOWN_ENEMY_USE_SKELETON_ORC_VAMPIRE_MALE_VAMPIRE_FEMALE_VAMPIRE_PAIR_OR);
            const bool pairs = command.target == server_constants::VAMPIRE_PAIR;
            if (command.amount == 0 || command.amount > 256 ||
                !world.spawnEnemies(players, playerId, found->second, static_cast<uint16_t>(command.amount), pairs, context.occupiedPositions)) {
                return reply(server_constants::CANNOT_SPAWN_ENEMIES_CHECK_YOUR_STATE_FREE_SPACE_AND_THE_ROOM_S);
            }
            return reply(server_constants::SPAWNED_PREFIX + std::to_string(command.amount * (pairs ? 2 : 1)) + server_constants::ENEMIES_ON_THIS_FLOOR_SUFFIX);
        }
        case CheatAction::KillEnemy:
        case CheatAction::KillEnemies: {
            if (player.getZone() != Zone::Dungeon)
                return reply(server_constants::ENTER_THE_MINE_FIRST_CHEAT_WARP_MINE_1);
            const auto killed = world.killEnemies(players, playerId, command.action == CheatAction::KillEnemy);
            return reply(server_constants::KILLED_PREFIX + std::to_string(killed) + server_constants::ENEMIES_ON_THIS_FLOOR_SUFFIX);
        }
        case CheatAction::EndDay:
            if (!endDay)
                return reply(server_constants::THE_DAY_CYCLE_IS_NOT_AVAILABLE);
            endDay();
            return reply(server_constants::TIME_ADVANCED_TO_THE_END_OF_THE_DAY_FOR_EVERYONE_CONFIRM_OK);
        case CheatAction::CompleteBundle:
        case CheatAction::CompleteAllBundles:
            return reply(server_constants::THE_COMMUNITY_CENTER_IS_NOT_AVAILABLE);
    }
    return reply(server_constants::UNKNOWN_CHEAT_USE_CHEATS_2);
}
