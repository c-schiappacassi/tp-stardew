#ifndef SERVER_WORLD_LOADING_MAP_DATA_H
#define SERVER_WORLD_LOADING_MAP_DATA_H

#include <optional>
#include <string>
#include <vector>

struct TilePosition {
    int x;
    int y;
};

struct FloorTile {
    int x;
    int y;
    int textureId;
    bool walkable;
};

struct MapElement {
    int x;
    int y;
    int id;
};

struct BuildingData {
    int x;
    int y;
    int id;
    TilePosition door;
};

struct ZoneData {
    int width;
    int height;
    std::vector<FloorTile> background;
    std::vector<BuildingData> buildings;
    std::vector<MapElement> decorations;
    std::vector<MapElement> frame;
    std::vector<TilePosition> lockedTiles;
    std::optional<TilePosition> doorTown;
    std::optional<TilePosition> doorFarm;
    std::optional<TilePosition> doorMines;
    std::optional<TilePosition> doorExit;
    std::optional<TilePosition> doorNextLevel;
    std::optional<int> playerHouses;
};

struct MapData {
    ZoneData farm;
    ZoneData town;
    std::vector<ZoneData> mineLevels;
};


#endif  // SERVER_WORLD_LOADING_MAP_DATA_H
