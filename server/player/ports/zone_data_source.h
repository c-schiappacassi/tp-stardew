#pragma once

class Player;
struct PlayerDataLocal;

class ZoneDataSource {
public:
    virtual void completeZoneData(const Player& player,PlayerDataLocal& data,bool includeBroken) const = 0;

    virtual ~ZoneDataSource() = default;
};