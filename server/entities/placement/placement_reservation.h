#pragma once

#include "entities/placement/placement.h"

class PlacementReservation {
private:
    Placement& placement;
    EntityId id;
    bool reserved;
    bool committed = false;

public:
    PlacementReservation(Placement& placement, EntityId id, const Position& position,
        const CollisionBox& collision, bool blocksMovement = true);
    ~PlacementReservation();

    PlacementReservation(const PlacementReservation&) = delete;
    PlacementReservation& operator=(const PlacementReservation&) = delete;

    bool isReserved() const;
    void commit();
};
