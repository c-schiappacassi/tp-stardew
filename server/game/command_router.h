#pragma once

#include <functional>
#include <map>
#include "dtos.h"

class Player;

class CommandRouter {
public:
    using Handler = std::function<void(Player&, const CommandDTO&)>;

    void bind(MessageType type, Handler handler);
    void dispatch(Player& player, const CommandDTO& command) const;

private:
    std::map<MessageType, Handler> handlers;
};