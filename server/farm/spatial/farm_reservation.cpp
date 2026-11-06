#include "farm_reservation.h"

FarmReservation::FarmReservation(FarmPlacement& placement, EntityId id, const Position& position,
        const CollisionBox& collision, bool blocksMovement):
    placement(placement), id(id),
    reserved(placement.reserve(id, position, collision, blocksMovement)) {}

FarmReservation::~FarmReservation() {
    if (reserved && !committed) {
        placement.release(id);
    }
}

bool FarmReservation::isReserved() const { return reserved; }

void FarmReservation::commit() { committed = true; }
