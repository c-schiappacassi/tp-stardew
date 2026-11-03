#pragma once

#include <cstdint>
#include <map>
#include "player/player_record.h"
#include "ports/zone_data_source.h"

class PlayerStatus {
private:
    const ZoneDataSource& zones;

    void fillPublicData(ServerStatus& status,const std::map<uint16_t, PlayerRecord>& players,const PlayerRecord& recipient) const;
    void completeLocalData(ServerStatus& status,const PlayerRecord& record,bool includeBroken) const;

public:
    explicit PlayerStatus(const ZoneDataSource& zones);

    ServerStatus buildForPlayer(const std::map<uint16_t, PlayerRecord>& players,const PlayerRecord& record) const;
    void publish(const std::map<uint16_t, PlayerRecord>& players,const ServerStatus& base) const;
};