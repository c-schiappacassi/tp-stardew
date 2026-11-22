#pragma once

#include <map>
#include <string>

#include "player/ports/zone_data_source.h"

#include "dtos.h"

class DayCycle;
class PlayerRecord;
class PlayerRoster;

class PlayerStatus {
private:
    const ZoneDataSource& zones;

    void fillPublicData(ServerStatus& status, const PlayerRoster& players) const;
    void completeLocalData(ServerStatus& status, const PlayerRecord& record, bool includeBroken,const DayCycle* day, bool multiplayer) const;

public:
    explicit PlayerStatus(const ZoneDataSource& zones);

    static ServerStatus registrationFailure(const std::string& message);
    std::map<std::string, PlayerDataLocal> getPersistencePlayers(const PlayerRoster& players) const;
    ServerStatus buildForPlayer(const PlayerRoster& players, const PlayerRecord& record,const DayCycle* day = nullptr) const;
    void publish(PlayerRoster& players, const ServerStatus& base,const DayCycle* day = nullptr) const;
};
