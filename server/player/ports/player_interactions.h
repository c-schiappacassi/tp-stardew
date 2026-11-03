#pragma once

class Player;
struct InteractDTO;

class PlayerInteractions {
public:
    virtual void interact(Player& player, const InteractDTO& interaction) = 0;

    virtual ~PlayerInteractions() = default;
};