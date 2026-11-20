#include "player_chat.h"

#include <exception>
#include <variant>
#include <vector>

#include "chat_command_parser.h"
#include "player/player_record.h"
#include "services/cheat_chat_service.h"
#include "services/community_chat_service.h"
#include "services/merchant_chat_service.h"

void PlayerChat::listCheats(PlayerRecord& sender) const {
    reply(sender, "/cheat health N | /cheat energy N (0 to 65535)");
    reply(sender, "/cheat add_gold N | /cheat remove_gold N (1 to 4294967295)");
    reply(sender, "/cheat complete_bundle identifier | /cheat complete_bundles");
    reply(sender, "/cheat next_level | /cheat care_crops | /cheat care_animals");
    if (!services.cheats) reply(sender, services.community ?
        "Only community center cheats are currently available." : "Cheats are not available yet.");
}

void PlayerChat::command(std::map<uint16_t, PlayerRecord>& players, PlayerRecord& sender,
                         const std::string& text, bool actionsBlocked) const {
    const auto parsed = ChatCommandParser::parse(text);
    if (const auto* error = std::get_if<ChatCommandError>(&parsed)) {
        reply(sender, error->text);
        return;
    }
    if (std::holds_alternative<ChatCheats>(parsed)) {
        listCheats(sender);
        return;
    }
    if (std::holds_alternative<ChatPlayers>(parsed)) {
        reply(sender, "Connected players:");
        for (const auto& [id, record] : players) {
            if (record.connected && record.output) reply(sender, record.name);
        }
        return;
    }

    const ChatCommandContext context{sender.id, sender.name, sender.player, actionsBlocked};
    std::vector<ChatMessageData> responses;
    try {
        if (const auto* request = std::get_if<TradeCommand>(&parsed)) {
            if (!services.merchants) {
                reply(sender, "Trading is not available yet.");
                return;
            }
            if (actionsBlocked || !sender.player.canAct()) {
                reply(sender, "You cannot trade right now.");
                return;
            }
            responses = services.merchants->execute(context, *request);
        } else if (const auto* request = std::get_if<CommunityCommand>(&parsed)) {
            if (!services.community) {
                reply(sender, "The community center is not available yet.");
                return;
            }
            responses = services.community->execute(context, *request);
        } else if (const auto* request = std::get_if<CheatCommand>(&parsed)) {
            if (services.community && (request->action == CheatAction::CompleteBundle ||
                                      request->action == CheatAction::CompleteAllBundles)) {
                responses = services.community->completeCheat(*request);
            } else {
                if (!services.cheats) {
                    reply(sender, "Cheats are not available yet.");
                    return;
                }
                responses = services.cheats->execute(context, *request);
            }
        }
    } catch (const std::exception&) {
        reply(sender, "Could not execute the command. Try again.");
        return;
    }
    sender.chatMessages.insert(sender.chatMessages.end(), responses.begin(), responses.end());
}
