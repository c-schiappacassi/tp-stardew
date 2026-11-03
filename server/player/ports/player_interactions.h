#pragma once

class Player;
struct InteractDTO;

class PlayerInteractions {
public:
    virtual void interact(Player& player, Direction direction) = 0;

    virtual ~PlayerInteractions() = default;
};