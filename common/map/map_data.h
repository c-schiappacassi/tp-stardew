#pragma once 

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

struct EditorBuildingData {
    int x;
    int y;
    int id;
    TilePosition door;
};

struct EditorZoneData {
    int width;
    int height;
    std::vector<FloorTile> background;
    std::vector<EditorBuildingData> buildings;
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
    EditorZoneData farm;
    EditorZoneData town;
    std::vector<EditorZoneData> mineLevels;
};