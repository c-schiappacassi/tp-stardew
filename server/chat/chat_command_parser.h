#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "parsed_chat_command.h"

class ChatCommandParser {
public:
    static ParsedChatCommand parse(const std::string& text);

private:
    static bool number(const std::string& text, uint32_t maximum, uint32_t& value, bool allowZero = false);
    static ParsedChatCommand trade(const std::vector<std::string>& tokens, TradeAction action);
    static ParsedChatCommand gift(const std::string& text, GiftAction action);
    static ParsedChatCommand cheat(const std::vector<std::string>& tokens);
};
