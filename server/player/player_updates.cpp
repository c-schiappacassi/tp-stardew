#include "player_updates.h"

PlayerUpdates::PlayerUpdates(const MovementRules& movement):movement(movement) {}

void PlayerUpdates::updateToolStates(std::map<uint16_t, PlayerRecord>& players,std::chrono::steady_clock::time_point now) const {
    for (auto& [id, record] : players) {
        if (record.connected) {
            record.player.updateToolState(now);
        }
    }
}

void PlayerUpdates::advancePlayers(std::map<uint16_t, PlayerRecord>& players,int normalStep) const {
    for (auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }

        Player& player = record.player;
        const int step = player.getMovementStep(normalStep);

        for (int distance = 0; distance < step; ++distance) {
            const Position next = player.getNextPosition(1);
            if (!movement.canMove(player, next)) {
                break;
            }

            player.advance(1);
        }
    }
}