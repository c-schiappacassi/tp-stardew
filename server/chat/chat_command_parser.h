#pragma once

#include <string>
#include <variant>

#include "chat_commands.h"

struct ChatCheats {};
struct ChatPlayers {};
struct ChatCommandError { std::string text; };

using ParsedChatCommand = std::variant<ChatCheats, ChatPlayers, TradeCommand, CheatCommand, ChatCommandError>;

class ChatCommandParser {
public:
    static ParsedChatCommand parse(const std::string& text);
};
