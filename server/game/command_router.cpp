#include "command_router.h"

#include <optional>

#include "player/player.h"
#include "player/ports/player_interactions.h"

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
}  // namespace

CommandRouter::CommandRouter(PlayerInteractions& interactions): interactions(interactions) {}

void CommandRouter::dispatch(Player& player, const CommandDTO& command, uint16_t playerId) const {
    switch (command.type()) {
        case MessageType::Move:
            player.setMovement(static_cast<const MoveDTO&>(command).dir);
            break;
        case MessageType::SelectSlot:
            player.selectSlot(static_cast<const SelectSlotDTO&>(command).slot);
            break;
        case MessageType::Interact: {
            const auto& interaction = static_cast<const InteractDTO&>(command);
            const auto direction = resolveInteractionDirection(player, interaction.dir);
            if (direction) {
                interactions.interact(player, *direction, playerId);
            }
            break;
        }
        default:
            break;
    }
}
