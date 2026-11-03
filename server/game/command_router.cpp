#include "command_router.h"

#include <utility>

void CommandRouter::bind(MessageType type, Handler handler) {
    handlers.insert_or_assign(type, std::move(handler));
}

void CommandRouter::dispatch(Player& player, const CommandDTO& command) const {
    const auto it = handlers.find(command.type());

    if (it != handlers.end()) {
        it->second(player, command);
    }
}