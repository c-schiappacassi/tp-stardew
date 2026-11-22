#include <exception>
#include <variant>
#include <vector>

#include "chat/chat_command_parser.h"
#include "chat/services/cheat_chat_service.h"
#include "chat/services/community_chat_service.h"
#include "chat/services/merchant_chat_service.h"
#include "player/sessions/player_roster.h"

#include "player_chat.h"

void PlayerChat::listCheats(PlayerRecord& sender) const {
    reply(sender, "/cheat health N | /cheat energy N (0 to 65535)");
    reply(sender, "/cheat add_gold N | /cheat remove_gold N (1 to 4294967295)");
    reply(sender, "/cheat complete_bundle identifier | /cheat complete_bundles");
    reply(sender, "/cheat next_level | /cheat care_crops | /cheat care_animals");
    reply(sender, "/cheat heal | /cheat end_day (affects all players)");
    reply(sender, "/cheat give product N [none|base|good|excellent|incredible|perfect]");
    reply(sender, "/cheat clear_slot N (slots start at 1) | /cheat spawn_animal cow|chicken N");
    reply(sender, "Animal colors: brown_cow, white_cow, brown_chicken, white_chicken, ""blue_chicken, void_chicken.");
    reply(sender, "/cheat warp farm|town|home|center|pierre|marnie|robin|clint|mine [level]");
    reply(sender,"/cheat spawn_enemy skeleton|orc|vampire_male|vampire_female|vampire_pair|skull [N]");
    reply(sender, "/cheat kill_enemy (nearest) | /cheat kill_enemies (current mine floor)");
    reply(sender,"Bundle names: crops, metals, animals, combat, money. Completing all ends the game.");
    if (!services.cheats)
        reply(sender, services.community ? "Only community center cheats are currently available." :"Cheats are not available yet.");
}

void PlayerChat::command(PlayerRoster& players, PlayerRecord& sender, const std::string& text,bool actionsBlocked) {
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
        for (const auto& [id, record]: players) {
            if (record.isConnected())
                reply(sender, record.getName());
        }
        return;
    }

    const ChatCommandContext context{sender.getId(), sender.getName(), manager, actionsBlocked};
    std::vector<ChatMessageData> responses;
    try {
        if (const auto* request = std::get_if<GiftCommand>(&parsed)) {
            gifts.execute(players, sender, *request, actionsBlocked);
            return;
        } else if (const auto* request = std::get_if<TradeCommand>(&parsed)) {
            if (!services.merchants) {
                reply(sender, "Trading is not available yet.");
                return;
            }
            if (actionsBlocked || !sender.getPlayer().canAct()) {
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
            if (actionsBlocked) {
                reply(sender, "Cheats cannot change the game after the day or game has ended.");
                return;
            }
            if (services.community && (request->action == CheatAction::CompleteBundle ||
                                       request->action == CheatAction::CompleteAllBundles)) {
                responses = services.community->completeCheat(*request);
            } else {
                if (!services.cheats) {
                    reply(sender, "Cheats are not available yet.");
                    return;
                }
                auto cheatContext = context;
                if (request->action == CheatAction::SpawnEnemy) {
                    for (const auto& [id, record]: players) {
                        if (record.isConnected() && record.getPlayer().getZone() == Zone::Dungeon &&
                            record.getPlayer().getDungeonLevel() ==sender.getPlayer().getDungeonLevel()) {
                            cheatContext.occupiedPositions.push_back(record.getPlayer().getPosition());
                        }
                    }
                }
                responses = services.cheats->execute(cheatContext, *request);
            }
        }
    } catch (const std::exception&) {
        reply(sender, "Could not execute the command. Try again.");
        return;
    }
    sender.enqueueMessages(responses);
}
