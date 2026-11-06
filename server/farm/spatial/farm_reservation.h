#pragma once

#include "farm/ports/farm_placement.h"

class FarmReservation {
private:
    FarmPlacement& placement;
    EntityId id;
    bool reserved;
    bool committed = false;

public:
    FarmReservation(FarmPlacement& placement, EntityId id, const Position& position,
        const CollisionBox& collision, bool blocksMovement = true);
    ~FarmReservation();

    FarmReservation(const FarmReservation&) = delete;
    FarmReservation& operator=(const FarmReservation&) = delete;

    bool isReserved() const;
    void commit();
};
