#pragma once

#include <string>
#include <variant>

#include "commands/cheat_command.h"
#include "commands/community_command.h"
#include "commands/gift_command.h"
#include "commands/trade_command.h"

struct ChatCheats {};
struct ChatPlayers {};

struct ChatCommandError {
    std::string text;
};

using ParsedChatCommand = std::variant<
    ChatCheats,
    ChatPlayers,
    GiftCommand,
    TradeCommand,
    CommunityCommand,
    CheatCommand,
    ChatCommandError>;
