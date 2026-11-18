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
        if (tokens.size() != 1) return ChatCommandError{"Uso: /consultar_catalogo"};
        return TradeCommand{action, {}, 0};
    }
    uint32_t quantity = 0;
    if (tokens.size() != 3 ||
        !number(tokens[2], std::numeric_limits<uint16_t>::max(), quantity)) {
        return ChatCommandError{action == TradeAction::Buy ?
            "Uso: /comprar producto cantidad (1 a 65535)" :
            "Uso: /vender producto cantidad (1 a 65535)"};
    }
    return TradeCommand{action, tokens[1], static_cast<uint16_t>(quantity)};
}

ParsedChatCommand cheat(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return ChatCommandError{"Usa /cheats para ver los comandos"};
    const auto& name = tokens[1];
    if (name == "vida" || name == "energia" || name == "agregar_oro" || name == "quitar_oro") {
        const bool vital = name == "vida" || name == "energia";
        const auto maximum = vital ? std::numeric_limits<uint16_t>::max() :std::numeric_limits<uint32_t>::max();
        uint32_t amount = 0;
        if (tokens.size() != 3 || !number(tokens[2], maximum, amount, vital)) {
            return ChatCommandError{"Uso: /cheat " + name + " cantidad (" + (vital ? "0 a 65535)" : "1 a 4294967295)")};
        }
        const auto action = name == "vida" ? CheatAction::SetLife :
                            name == "energia" ? CheatAction::SetEnergy :
                            name == "agregar_oro" ? CheatAction::AddGold : CheatAction::RemoveGold;
        return CheatCommand{action, amount, {}};
    }
    if (name == "completar_bundle") {
        if (tokens.size() != 3) {
            return ChatCommandError{"Uso: /cheat completar_bundle identificador"};
        }
        return CheatCommand{CheatAction::CompleteBundle, 0, tokens[2]};
    }
    const std::pair<std::string_view, CheatAction> actions[]{
        {"completar_bundles", CheatAction::CompleteAllBundles},
        {"siguiente_nivel", CheatAction::NextLevel},
        {"cuidar_cultivos", CheatAction::CareCrops},
        {"cuidar_animales", CheatAction::CareAnimals},
    };
    for (const auto& [keyword, action] : actions) {
        if (name == keyword) {
            if (tokens.size() != 2) return ChatCommandError{"Uso: /cheat " + name};
            return CheatCommand{action, 0, {}};
        }
    }
    return ChatCommandError{"Cheat desconocido. Usa /cheats"};
}
}

ParsedChatCommand ChatCommandParser::parse(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream input(text);
    for (std::string token; input >> token;) tokens.push_back(std::move(token));
    if (tokens.empty()) return ChatCommandError{"Comando vacio"};
    const auto& command = tokens.front();
    if (command == "/cheats") {
        if (tokens.size() != 1) return ChatCommandError{"Uso: /cheats"};
        return ChatCheats{};
    }
    if (command == "/jugadores") {
        if (tokens.size() != 1) return ChatCommandError{"Uso: /jugadores"};
        return ChatPlayers{};
    }
    if (command == "/consultar_catalogo") return trade(tokens, TradeAction::Catalog);
    if (command == "/comprar") return trade(tokens, TradeAction::Buy);
    if (command == "/vender") return trade(tokens, TradeAction::Sell);
    if (command == "/cheat") return cheat(tokens);
    return ChatCommandError{"Comando desconocido"};
}
