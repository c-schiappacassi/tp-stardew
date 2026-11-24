#pragma once

class PlayerSnapshot;
class Position;

class MovementRules {
public:
    virtual bool canMove(const PlayerSnapshot& player, const Position& next) const = 0;

    virtual ~MovementRules();
};
