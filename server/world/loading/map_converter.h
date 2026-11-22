#ifndef SERVER_WORLD_LOADING_MAP_CONVERTER_H
#define SERVER_WORLD_LOADING_MAP_CONVERTER_H

#include <cstdint>
#include <string>
#include <vector>

#include "game/game_setup.h"
#include "map_data.h"

class MapConverter {
public:
    GameSetup convert(const MapData& data, uint8_t worldId) const;

private:
    static ZoneMap convertTerrain(const ZoneData& zone, const std::string& name);
    static Position convertPosition(const ZoneMap& terrain, const TilePosition& tile,
                                    const std::string& field);
    static Position findSpawn(const ZoneMap& terrain);
    static FarmLayout convertFarm(const ZoneData& zone);
    static InteriorId townInteriorId(int buildingId);
    static TownBuildingConfig convertTownBuilding(const BuildingData& building);
    static TownConfig convertTown(const ZoneData& zone, std::vector<InteriorLayout>& interiors);
    static DungeonRoomLayout convertRoom(const ZoneData& zone, uint8_t level);
};

#endif
