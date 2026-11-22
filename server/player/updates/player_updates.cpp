#include "player_updates.h"

#include "player/ports/movement_rules.h"
#include "player/sessions/player_roster.h"

PlayerUpdates::PlayerUpdates(const MovementRules& movement): movement(movement) {}

void PlayerUpdates::updateToolStates(PlayerRoster& players,std::chrono::steady_clock::time_point now) const {
    for (auto& [id, record]: players) {
        if (record.isConnected()) {
            record.getPlayer().updateToolState(now);
        }
    }
}

void PlayerUpdates::advancePlayers(PlayerRoster& players, int normalStep) const {
    for (auto& [id, record]: players) {
        if (!record.isConnected()) {
            continue;
        }

        Player& player = record.getPlayer();
        const int step = player.getMovementStep(normalStep);
        const auto snapshot = player.snapshot();

        for (int distance = 0; distance < step; ++distance) {
            const Position next = player.getNextPosition(1);
            if (!movement.canMove(snapshot, next)) {
                break;
            }

            player.advance(1);
        }
    }
}
