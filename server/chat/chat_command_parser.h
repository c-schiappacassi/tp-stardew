#pragma once

#include <string>

#include "parsed_chat_command.h"

class ChatCommandParser {
public:
    static ParsedChatCommand parse(const std::string& text);
};
