#include "player_actions.h"

#include "player.h"

#include <optional>

namespace {
std::optional<Direction> resolveInteractionDirection(const Player& player, Direction requested) {
    switch (requested) {
        case Direction::None:
            return player.getPublicData().dir;
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            return requested;
        default:
            return std::nullopt;
    }
}
}

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
            const auto& interaction = static_cast<const InteractDTO&>(command);
            if (const auto direction = resolveInteractionDirection(player, interaction.dir)) {
                world->interact(player, *direction);
            }
        });
}

void PlayerActions::execute(Player& player, const CommandDTO& command) const {
    router.dispatch(player, command);
}