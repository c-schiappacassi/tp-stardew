#include "combat_progress.h"

void CombatProgress::newVisit() { players.clear(); }

void CombatProgress::defeat(uint16_t playerId, uint8_t room, const Enemy& enemy) {
    auto& evidence = players[playerId];
    switch (enemy.getType()) {
        case EnemyType::Skeleton:
            evidence.skeletonRooms.insert(room);
            for (uint8_t first = 1; first <= 8; ++first) {
                if (evidence.skeletonRooms.contains(first) && evidence.skeletonRooms.contains(first + 1) && evidence.skeletonRooms.contains(first + 2)) {
                    completed.skeletons = true;
                }
            }
            break;
        case EnemyType::Orc:
            if (enemy.getGroupId() != 0) {
                evidence.groups[enemy.getGroupId()].insert(enemy.getId());
                if (evidence.groups[enemy.getGroupId()].size() >= 4) {
                    completed.orcs = true;
                }
            }
            break;
        case EnemyType::VampireMale:
        case EnemyType::VampireFemale:
            if (enemy.getPairId() != 0) {
                evidence.pairs[enemy.getPairId()].insert(enemy.getId());
                if (evidence.pairs[enemy.getPairId()].size() == 2) {
                    completed.vampires = true;
                }
            }
            break;
        default:
            break;
    }
}

void CombatProgress::enter(uint16_t playerId, uint8_t room, bool skull) {
    auto& evidence = players[playerId];
    evidence.room = room;
    evidence.damaged = false;
    evidence.sawSkull = skull;
}

void CombatProgress::damage(uint16_t playerId) { players[playerId].damaged = true; }

void CombatProgress::observeSkull(uint16_t playerId, uint8_t room) {
    const auto found = players.find(playerId);
    if (found != players.end() && found->second.room == room)
        found->second.sawSkull = true;
}

void CombatProgress::leave(uint16_t playerId, bool survived) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        return;
    }
    auto& evidence = it->second;
    if (survived && evidence.room != 0 && evidence.sawSkull && !evidence.damaged) {
        completed.skull = true;
    }
    evidence.room = 0;
}

const CombatProgressData& CombatProgress::data() const { return completed; }

bool CombatProgress::complete() const { return completed.skeletons && completed.orcs && completed.vampires && completed.skull; }
