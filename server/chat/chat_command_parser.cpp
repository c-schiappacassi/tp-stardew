#include "chat_command_parser.h"

#include "server_constants.h"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

bool ChatCommandParser::number(const std::string& text, uint32_t maximum, uint32_t& value, bool allowZero) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](char ch) {
            return ch >= '0' && ch <= '9';
        })) return false;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size() &&
           value <= maximum && (allowZero || value > 0);
}

ParsedChatCommand ChatCommandParser::trade(const std::vector<std::string>& tokens, TradeAction action) {
    if (action == TradeAction::Catalog) {
        if (tokens.size() != 1) return ChatCommandError{server_constants::USAGE_CATALOG};
        return TradeCommand{action, {}, 0};
    }
    uint32_t quantity = 0;
    if (tokens.size() != 3 ||
        !number(tokens[2], std::numeric_limits<uint16_t>::max(), quantity)) {
        return ChatCommandError{action == TradeAction::Buy ?
            server_constants::USAGE_BUY_PRODUCT_AMOUNT_1_TO_65535 :
            server_constants::USAGE_SELL_PRODUCT_AMOUNT_1_TO_65535};
    }
    return TradeCommand{action, tokens[1], static_cast<uint16_t>(quantity)};
}

ParsedChatCommand ChatCommandParser::gift(const std::string& text, GiftAction action) {
    const std::string usage = action == GiftAction::Offer ? server_constants::USAGE_GIFT_CHARACTER_NAME_AMOUNT_1_TO_65535 :
        action == GiftAction::Accept ? server_constants::USAGE_ACCEPT_CHARACTER_NAME : server_constants::USAGE_REJECT_CHARACTER_NAME;
    std::istringstream input(text);
    std::string command;
    std::string player;
    input >> command >> std::ws;
    if (input.get() != '@' || !(input >> std::quoted(player)) || player.empty())
        return ChatCommandError{usage};

    uint32_t quantity = 1;
    std::string amount;
    std::string extra;
    if (input >> amount) {
        if (action != GiftAction::Offer || !number(amount, std::numeric_limits<uint16_t>::max(), quantity) || (input >> extra))
            return ChatCommandError{usage};
    }
    return GiftCommand{action, std::move(player), static_cast<uint16_t>(quantity)};
}

ParsedChatCommand ChatCommandParser::cheat(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return ChatCommandError{server_constants::USE_CHEATS_TO_SEE_THE_COMMANDS};
    const auto& name = tokens[1];
    if (name == server_constants::SPAWN_ENEMY) {
        uint32_t amount = 1;
        if ((tokens.size() != 3 && tokens.size() != 4) ||
            (tokens.size() == 4 && !number(tokens[3], 256, amount))) {
            return ChatCommandError{server_constants::USAGE_CHEAT_SPAWN_ENEMY_SKELETON_ORC_VAMPIRE_MALE_VAMPIRE_FEMALE_VAMPIRE_PAIR};
        }
        return CheatCommand{CheatAction::SpawnEnemy, amount, tokens[2]};
    }
    if (name == server_constants::GIVE) {
        uint32_t amount = 0;
        if ((tokens.size() != 4 && tokens.size() != 5) ||
            !number(tokens[3], std::numeric_limits<uint16_t>::max(), amount)) {
            return ChatCommandError{server_constants::USAGE_CHEAT_GIVE_PRODUCT_AMOUNT_NONE_BASE_GOOD_EXCELLENT_INCREDIBLE_PERFECT};
        }
        Quality quality = Quality::None;
        if (tokens.size() == 5) {
            const std::pair<std::string_view, Quality> qualities[]{
                {server_constants::NONE_2, Quality::None}, {server_constants::BASE, Quality::Base},
                {server_constants::GOOD, Quality::Good}, {server_constants::EXCELLENT, Quality::Excellent},
                {server_constants::INCREDIBLE, Quality::Incredible}, {server_constants::PERFECT, Quality::Perfect}
            };
            const auto found = std::find_if(std::begin(qualities), std::end(qualities),
                [&](const auto& entry) { return entry.first == tokens[4]; });
            if (found == std::end(qualities)) return ChatCommandError{server_constants::UNKNOWN_QUALITY_USE_NONE_BASE_GOOD_EXCELLENT_INCREDIBLE_OR_PERFECT};
            quality = found->second;
        }
        return CheatCommand{CheatAction::GiveItem, amount, tokens[2], quality};
    }
    if (name == server_constants::SPAWN_ANIMAL || name == server_constants::CLEAR_SLOT) {
        const bool animal = name == server_constants::SPAWN_ANIMAL;
        uint32_t amount = 0;
        if (tokens.size() != (animal ? 4u : 3u) ||
            !number(tokens.back(), animal ? std::numeric_limits<uint16_t>::max() :
                                           std::numeric_limits<uint8_t>::max(), amount)) {
            return ChatCommandError{animal ? server_constants::USAGE_CHEAT_SPAWN_ANIMAL_COW_CHICKEN_WHITE_COW_BLUE_CHICKEN_AMOUNT_1 :
                                            server_constants::USAGE_CHEAT_CLEAR_SLOT_SLOT_1_TO_INVENTORY_CAPACITY};
        }
        return CheatCommand{animal ? CheatAction::SpawnAnimal : CheatAction::ClearSlot,
                            amount, animal ? tokens[2] : server_constants::EMPTY};
    }
    if (name == server_constants::WARP) {
        uint32_t level = 1;
        if (tokens.size() < 3 || tokens.size() > 4 ||
            (tokens.size() == 4 && (tokens[2] != server_constants::MINE ||
                !number(tokens[3], std::numeric_limits<uint8_t>::max(), level)))) {
            return ChatCommandError{server_constants::USAGE_CHEAT_WARP_FARM_TOWN_HOME_CENTER_PIERRE_MARNIE_ROBIN_CLINT_MINE};
        }
        return CheatCommand{CheatAction::Warp, level, tokens[2]};
    }
    if (name == server_constants::HEALTH_2 || name == server_constants::ENERGY_2 || name == server_constants::ADD_GOLD || name == server_constants::REMOVE_GOLD) {
        const bool vital = name == server_constants::HEALTH_2 || name == server_constants::ENERGY_2;
        const auto maximum = vital ? std::numeric_limits<uint16_t>::max() :std::numeric_limits<uint32_t>::max();
        uint32_t amount = 0;
        if (tokens.size() != 3 || !number(tokens[2], maximum, amount, vital)) {
            return ChatCommandError{server_constants::USAGE_CHEAT_PREFIX + name + server_constants::AMOUNT_SUFFIX + (vital ? server_constants::TEXT_0_TO_65535 : server_constants::TEXT_1_TO_4294967295)};
        }
        const auto action = name == server_constants::HEALTH_2 ? CheatAction::SetLife :
                            name == server_constants::ENERGY_2 ? CheatAction::SetEnergy :
                            name == server_constants::ADD_GOLD ? CheatAction::AddGold : CheatAction::RemoveGold;
        return CheatCommand{action, amount, {}};
    }
    if (name == server_constants::COMPLETE_BUNDLE) {
        if (tokens.size() != 3) {
            return ChatCommandError{server_constants::USAGE_CHEAT_COMPLETE_BUNDLE_IDENTIFIER};
        }
        return CheatCommand{CheatAction::CompleteBundle, 0, tokens[2]};
    }
    const std::pair<std::string_view, CheatAction> actions[]{
        {server_constants::ITEMS, CheatAction::ListItems},
        {server_constants::COMPLETE_BUNDLES, CheatAction::CompleteAllBundles},
        {server_constants::NEXT_LEVEL, CheatAction::NextLevel},
        {server_constants::CARE_CROPS, CheatAction::CareCrops},
        {server_constants::CARE_ANIMALS, CheatAction::CareAnimals},
        {server_constants::HEAL, CheatAction::Heal},
        {server_constants::END_DAY, CheatAction::EndDay},
        {server_constants::KILL_ENEMY, CheatAction::KillEnemy},
        {server_constants::KILL_ENEMIES, CheatAction::KillEnemies},
    };
    for (const auto& [keyword, action] : actions) {
        if (name == keyword) {
            if (tokens.size() != 2) return ChatCommandError{server_constants::USAGE_CHEAT_PREFIX + name};
            return CheatCommand{action, 0, {}};
        }
    }
    return ChatCommandError{server_constants::UNKNOWN_CHEAT_USE_CHEATS};
}

ParsedChatCommand ChatCommandParser::parse(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream input(text);
    for (std::string token; input >> token;) tokens.push_back(std::move(token));
    if (tokens.empty()) return ChatCommandError{server_constants::EMPTY_COMMAND};
    const auto& command = tokens.front();
    if (command == server_constants::COMMAND_CHEATS) {
        if (tokens.size() != 1) return ChatCommandError{server_constants::USAGE_CHEATS};
        return ChatCheats{};
    }
    if (command == server_constants::COMMAND_PLAYERS) {
        if (tokens.size() != 1) return ChatCommandError{server_constants::USAGE_PLAYERS};
        return ChatPlayers{};
    }
    if (command == server_constants::COMMAND_GIFT) return gift(text, GiftAction::Offer);
    if (command == server_constants::COMMAND_ACCEPT) return gift(text, GiftAction::Accept);
    if (command == server_constants::COMMAND_REJECT) return gift(text, GiftAction::Reject);
    if (command == server_constants::COMMAND_CANCEL_GIFT) {
        if (tokens.size() != 1) return ChatCommandError{server_constants::USAGE_CANCEL_GIFT};
        return GiftCommand{GiftAction::Cancel, {}};
    }
    if (command == server_constants::COMMAND_CENTER) {
        if (tokens.size() > 2) return ChatCommandError{server_constants::USAGE_CENTER_CROPS_METALS_ANIMALS_COMBAT_MONEY};
        return CommunityCommand{CommunityStatusCommand{tokens.size() == 2 ? tokens[1] : server_constants::EMPTY}};
    }
    if (command == server_constants::COMMAND_REWARDS) {
        if (tokens.size() != 1) return ChatCommandError{server_constants::USAGE_REWARDS};
        return CommunityCommand{CommunityRewardsCommand{}};
    }
    if (command == server_constants::COMMAND_DONATE || command == server_constants::COMMAND_DONATE_COINS) {
        const bool money = command == server_constants::COMMAND_DONATE_COINS;
        uint32_t quantity = 0;
        if (tokens.size() != (money ? 2u : 3u) ||
            !number(tokens.back(), std::numeric_limits<uint32_t>::max(), quantity)) {
            return ChatCommandError{money ? server_constants::USAGE_DONATE_COINS_AMOUNT_GREATER_THAN_ZERO :
                                            server_constants::USAGE_DONATE_PRODUCT_AMOUNT_GREATER_THAN_ZERO};
        }
        if (money) return CommunityCommand{DonateCoinsCommand{quantity}};
        return CommunityCommand{DonateItemCommand{tokens[1], quantity}};
    }
    if (command == server_constants::COMMAND_CATALOG) return trade(tokens, TradeAction::Catalog);
    if (command == server_constants::COMMAND_BUY) return trade(tokens, TradeAction::Buy);
    if (command == server_constants::COMMAND_SELL) return trade(tokens, TradeAction::Sell);
    if (command == server_constants::COMMAND_CHEAT) return cheat(tokens);
    return ChatCommandError{server_constants::UNKNOWN_COMMAND};
}
