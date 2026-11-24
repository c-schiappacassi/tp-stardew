#pragma once

class PlayerSnapshot;
struct PlayerDataLocal;

class ZoneDataSource {
public:
    virtual void completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data,bool includeBroken) const = 0;

    virtual ~ZoneDataSource();
};
