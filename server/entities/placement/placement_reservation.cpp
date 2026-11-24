#include "placement_reservation.h"

PlacementReservation::PlacementReservation(Placement& placement, EntityId id, const Position& position, const CollisionBox& collision, bool blocksMovement):
    placement(placement), id(id), reserved(placement.reserve(id, position, collision, blocksMovement)) {}

PlacementReservation::~PlacementReservation() {
    if (reserved && !committed) {
        placement.release(id);
    }
}

bool PlacementReservation::isReserved() const { return reserved; }

void PlacementReservation::commit() { committed = true; }
