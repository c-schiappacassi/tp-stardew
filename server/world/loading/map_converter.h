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
    ConstMapData convertVisuals(const MapData& data) const;

private:
    static BackgroundData convertVisualZone(const EditorZoneData& zone, const std::string& name);
    static ZoneMap convertTerrain(const EditorZoneData& zone, const std::string& name);
    static Position convertPosition(const ZoneMap& terrain, const TilePosition& tile,
                                    const std::string& field);
    static Position findSpawn(const ZoneMap& terrain);
    static FarmLayout convertFarm(const EditorZoneData& zone);
    static InteriorId townInteriorId(int buildingId);
    static TownBuildingConfig convertTownBuilding(const EditorBuildingData& building);
    static TownConfig convertTown(const EditorZoneData& zone, std::vector<InteriorLayout>& interiors);
    static DungeonRoomLayout convertRoom(const EditorZoneData& zone, uint8_t level);
};

#endif
