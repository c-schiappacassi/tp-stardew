#pragma once

class Player;
class Position;

class MovementRules {
public:
    virtual bool canMove(const Player& player, const Position& next) const = 0;

    virtual ~MovementRules() = default;
};