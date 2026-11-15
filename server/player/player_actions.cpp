#include "player_actions.h"

PlayerActions::PlayerActions(PlayerInteractions& interactions): router(interactions) {}

void PlayerActions::execute(Player& player, const CommandDTO& command, uint16_t playerId) const {
    router.dispatch(player, command, playerId);
}
