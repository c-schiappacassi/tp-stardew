#include "combat_progress.h"

void CombatProgress::newVisit() { players.clear(); }

void CombatProgress::defeat(uint16_t playerId, uint8_t room, const Enemy& enemy) {
    auto& evidence = players[playerId];
    switch (enemy.data.type) {
        case EnemyType::Skeleton:
            evidence.skeletonRooms.insert(room);
            for (uint8_t first = 1; first <= 8; ++first) {
                if (evidence.skeletonRooms.contains(first) &&evidence.skeletonRooms.contains(first + 1) &&evidence.skeletonRooms.contains(first + 2)) {
                    completed.skeletons = true;
                }
            }
            break;
        case EnemyType::Orc:
            if (enemy.groupId != 0) {
                evidence.groups[enemy.groupId].insert(enemy.data.id);
                if (evidence.groups[enemy.groupId].size() >= 4) {
                    completed.orcs = true;
                }
            }
            break;
        case EnemyType::VampireMale:
        case EnemyType::VampireFemale:
            if (enemy.pairId != 0) {
                evidence.pairs[enemy.pairId].insert(enemy.data.id);
                if (evidence.pairs[enemy.pairId].size() == 2) {
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

bool CombatProgress::complete() const {
    return completed.skeletons && completed.orcs && completed.vampires && completed.skull;
}
