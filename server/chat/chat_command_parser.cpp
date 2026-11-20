#include "chat_command_parser.h"

#include <algorithm>
#include <charconv>
#include <limits>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace {
bool number(const std::string& text, uint32_t maximum, uint32_t& value, bool allowZero = false) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](char ch) {
            return ch >= '0' && ch <= '9';
        })) return false;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size() &&
           value <= maximum && (allowZero || value > 0);
}

ParsedChatCommand trade(const std::vector<std::string>& tokens, TradeAction action) {
    if (action == TradeAction::Catalog) {
        if (tokens.size() != 1) return ChatCommandError{"Usage: /catalog"};
        return TradeCommand{action, {}, 0};
    }
    uint32_t quantity = 0;
    if (tokens.size() != 3 ||
        !number(tokens[2], std::numeric_limits<uint16_t>::max(), quantity)) {
        return ChatCommandError{action == TradeAction::Buy ?
            "Usage: /buy product amount (1 to 65535)" :
            "Usage: /sell product amount (1 to 65535)"};
    }
    return TradeCommand{action, tokens[1], static_cast<uint16_t>(quantity)};
}

ParsedChatCommand cheat(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return ChatCommandError{"Use /cheats to see the commands"};
    const auto& name = tokens[1];
    if (name == "give") {
        uint32_t amount = 0;
        if ((tokens.size() != 4 && tokens.size() != 5) ||
            !number(tokens[3], std::numeric_limits<uint16_t>::max(), amount)) {
            return ChatCommandError{"Usage: /cheat give product amount [none|base|good|excellent|incredible|perfect]"};
        }
        Quality quality = Quality::None;
        if (tokens.size() == 5) {
            const std::pair<std::string_view, Quality> qualities[]{
                {"none", Quality::None}, {"base", Quality::Base},
                {"good", Quality::Good}, {"excellent", Quality::Excellent},
                {"incredible", Quality::Incredible}, {"perfect", Quality::Perfect}
            };
            const auto found = std::find_if(std::begin(qualities), std::end(qualities),
                [&](const auto& entry) { return entry.first == tokens[4]; });
            if (found == std::end(qualities)) return ChatCommandError{"Unknown quality. Use none, base, good, excellent, incredible or perfect."};
            quality = found->second;
        }
        return CheatCommand{CheatAction::GiveItem, amount, tokens[2], quality};
    }
    if (name == "spawn_animal" || name == "clear_slot") {
        const bool animal = name == "spawn_animal";
        uint32_t amount = 0;
        if (tokens.size() != (animal ? 4u : 3u) ||
            !number(tokens.back(), animal ? std::numeric_limits<uint16_t>::max() :
                                           std::numeric_limits<uint8_t>::max(), amount)) {
            return ChatCommandError{animal ? "Usage: /cheat spawn_animal cow|chicken amount (1 to 65535)" :
                                            "Usage: /cheat clear_slot slot (1 to inventory capacity)"};
        }
        return CheatCommand{animal ? CheatAction::SpawnAnimal : CheatAction::ClearSlot,
                            amount, animal ? tokens[2] : ""};
    }
    if (name == "warp") {
        uint32_t level = 1;
        if (tokens.size() < 3 || tokens.size() > 4 ||
            (tokens.size() == 4 && (tokens[2] != "mine" ||
                !number(tokens[3], std::numeric_limits<uint8_t>::max(), level)))) {
            return ChatCommandError{"Usage: /cheat warp farm|town|home|center|pierre|marnie|robin|clint|mine [level]"};
        }
        return CheatCommand{CheatAction::Warp, level, tokens[2]};
    }
    if (name == "health" || name == "energy" || name == "add_gold" || name == "remove_gold") {
        const bool vital = name == "health" || name == "energy";
        const auto maximum = vital ? std::numeric_limits<uint16_t>::max() :std::numeric_limits<uint32_t>::max();
        uint32_t amount = 0;
        if (tokens.size() != 3 || !number(tokens[2], maximum, amount, vital)) {
            return ChatCommandError{"Usage: /cheat " + name + " amount (" + (vital ? "0 to 65535)" : "1 to 4294967295)")};
        }
        const auto action = name == "health" ? CheatAction::SetLife :
                            name == "energy" ? CheatAction::SetEnergy :
                            name == "add_gold" ? CheatAction::AddGold : CheatAction::RemoveGold;
        return CheatCommand{action, amount, {}};
    }
    if (name == "complete_bundle") {
        if (tokens.size() != 3) {
            return ChatCommandError{"Usage: /cheat complete_bundle identifier"};
        }
        return CheatCommand{CheatAction::CompleteBundle, 0, tokens[2]};
    }
    const std::pair<std::string_view, CheatAction> actions[]{
        {"complete_bundles", CheatAction::CompleteAllBundles},
        {"next_level", CheatAction::NextLevel},
        {"care_crops", CheatAction::CareCrops},
        {"care_animals", CheatAction::CareAnimals},
        {"heal", CheatAction::Heal},
        {"end_day", CheatAction::EndDay},
    };
    for (const auto& [keyword, action] : actions) {
        if (name == keyword) {
            if (tokens.size() != 2) return ChatCommandError{"Usage: /cheat " + name};
            return CheatCommand{action, 0, {}};
        }
    }
    return ChatCommandError{"Unknown cheat. Use /cheats"};
}
}

ParsedChatCommand ChatCommandParser::parse(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream input(text);
    for (std::string token; input >> token;) tokens.push_back(std::move(token));
    if (tokens.empty()) return ChatCommandError{"Empty command"};
    const auto& command = tokens.front();
    if (command == "/cheats") {
        if (tokens.size() != 1) return ChatCommandError{"Usage: /cheats"};
        return ChatCheats{};
    }
    if (command == "/players") {
        if (tokens.size() != 1) return ChatCommandError{"Usage: /players"};
        return ChatPlayers{};
    }
    if (command == "/center") {
        if (tokens.size() > 2) return ChatCommandError{"Usage: /center [crops|metals|animals|combat|money]"};
        return CommunityCommand{CommunityStatusCommand{tokens.size() == 2 ? tokens[1] : ""}};
    }
    if (command == "/rewards") {
        if (tokens.size() != 1) return ChatCommandError{"Usage: /rewards"};
        return CommunityCommand{CommunityRewardsCommand{}};
    }
    if (command == "/donate" || command == "/donate_coins") {
        const bool money = command == "/donate_coins";
        uint32_t quantity = 0;
        if (tokens.size() != (money ? 2u : 3u) ||
            !number(tokens.back(), std::numeric_limits<uint32_t>::max(), quantity)) {
            return ChatCommandError{money ? "Usage: /donate_coins amount (greater than zero)" :
                                            "Usage: /donate product amount (greater than zero)"};
        }
        if (money) return CommunityCommand{DonateCoinsCommand{quantity}};
        return CommunityCommand{DonateItemCommand{tokens[1], quantity}};
    }
    if (command == "/catalog") return trade(tokens, TradeAction::Catalog);
    if (command == "/buy") return trade(tokens, TradeAction::Buy);
    if (command == "/sell") return trade(tokens, TradeAction::Sell);
    if (command == "/cheat") return cheat(tokens);
    return ChatCommandError{"Unknown command"};
}
