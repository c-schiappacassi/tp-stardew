#include "player_actions.h"

#include "player.h"

PlayerActions::PlayerActions(PlayerInteractions& interactions) {
    router.bind(MessageType::Move,
        [](Player& player, const CommandDTO& command) {
            player.setMovement(
                static_cast<const MoveDTO&>(command).dir);
        });

    router.bind(MessageType::SelectSlot,
        [](Player& player, const CommandDTO& command) {
            player.selectSlot(
                static_cast<const SelectSlotDTO&>(command).slot);
        });

    router.bind(MessageType::Interact,
        [world = &interactions](
            Player& player, const CommandDTO& command) {
            world->interact(
                player, static_cast<const InteractDTO&>(command));
        });
}

void PlayerActions::execute(Player& player, const CommandDTO& command) const {
    router.dispatch(player, command);
}