#include "server_constants.h"

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
    reply(sender, server_constants::COMMAND_CHEAT_HEALTH_N_CHEAT_ENERGY_N_0_TO_65535);
    reply(sender, server_constants::COMMAND_CHEAT_ADD_GOLD_N_CHEAT_REMOVE_GOLD_N_1_TO_4294967295);
    reply(sender, server_constants::COMMAND_CHEAT_COMPLETE_BUNDLE_IDENTIFIER_CHEAT_COMPLETE_BUNDLES);
    reply(sender, server_constants::COMMAND_CHEAT_NEXT_LEVEL_CHEAT_CARE_CROPS_CHEAT_CARE_ANIMALS);
    reply(sender, server_constants::COMMAND_CHEAT_HEAL_CHEAT_END_DAY_AFFECTS_ALL_PLAYERS);
    reply(sender, server_constants::COMMAND_CHEAT_GIVE_PRODUCT_N_NONE_BASE_GOOD_EXCELLENT_INCREDIBLE_PERFECT);
    reply(sender, server_constants::COMMAND_CHEAT_ITEMS_HELP);
    reply(sender, server_constants::COMMAND_CHEAT_CLEAR_SLOT_N_SLOTS_START_AT_1_CHEAT_SPAWN_ANIMAL_COW);
    reply(sender, server_constants::ANIMAL_COLORS_BROWN_COW_WHITE_COW_BROWN_CHICKEN_WHITE_CHICKEN_BLUE_CHICKEN);
    reply(sender, server_constants::COMMAND_CHEAT_WARP_FARM_TOWN_HOME_CENTER_PIERRE_MARNIE_ROBIN_CLINT_MINE_LEVEL);
    reply(sender,server_constants::COMMAND_CHEAT_SPAWN_ENEMY_SKELETON_ORC_VAMPIRE_MALE_VAMPIRE_FEMALE_VAMPIRE_PAIR_SKULL);
    reply(sender, server_constants::COMMAND_CHEAT_KILL_ENEMY_NEAREST_CHEAT_KILL_ENEMIES_CURRENT_MINE_FLOOR);
    reply(sender,server_constants::BUNDLE_NAMES_CROPS_METALS_ANIMALS_COMBAT_MONEY_COMPLETING_ALL_ENDS_THE_GAME);
    if (!services.cheats)
        reply(sender, services.community ? server_constants::ONLY_COMMUNITY_CENTER_CHEATS_ARE_CURRENTLY_AVAILABLE :server_constants::CHEATS_ARE_NOT_AVAILABLE_YET);
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
        reply(sender, server_constants::CONNECTED_PLAYERS);
        for (const auto& [id, record]: players) {
            if (record.isConnected())
                reply(sender, record.getPlayer().getCharacterName());
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
                reply(sender, server_constants::TRADING_IS_NOT_AVAILABLE_YET);
                return;
            }
            if (actionsBlocked || !sender.getPlayer().canAct()) {
                reply(sender, server_constants::YOU_CANNOT_TRADE_RIGHT_NOW);
                return;
            }
            responses = services.merchants->execute(context, *request);
        } else if (const auto* request = std::get_if<CommunityCommand>(&parsed)) {
            if (!services.community) {
                reply(sender, server_constants::THE_COMMUNITY_CENTER_IS_NOT_AVAILABLE_YET);
                return;
            }
            responses = services.community->execute(context, *request);
        } else if (const auto* request = std::get_if<CheatCommand>(&parsed)) {
            if (actionsBlocked && request->action != CheatAction::ListItems) {
                reply(sender, server_constants::CHEATS_CANNOT_CHANGE_THE_GAME_AFTER_THE_DAY_OR_GAME_HAS_ENDED);
                return;
            }
            if (services.community && (request->action == CheatAction::CompleteBundle ||
                                       request->action == CheatAction::CompleteAllBundles)) {
                responses = services.community->completeCheat(*request);
            } else {
                if (!services.cheats) {
                    reply(sender, server_constants::CHEATS_ARE_NOT_AVAILABLE_YET);
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
        reply(sender, server_constants::COULD_NOT_EXECUTE_THE_COMMAND_TRY_AGAIN);
        return;
    }
    sender.enqueueMessages(responses);
}
