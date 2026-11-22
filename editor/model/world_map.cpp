#include "world_map.h"

namespace {
const int FARM_WIDTH = 64;
const int FARM_HEIGHT = 48;
const int TOWN_WIDTH = 64;
const int TOWN_HEIGHT = 48;
const int MINE_WIDTH = 32;
const int MINE_HEIGHT = 32;
const int MINE_LEVELS = 10;
}

WorldMap::WorldMap():
        farm(FARM_WIDTH, FARM_HEIGHT, TileTextureId::Grass),
        town(TOWN_WIDTH, TOWN_HEIGHT, TileTextureId::Grass),
        mineLevels(MINE_LEVELS, ZoneMap(MINE_WIDTH, MINE_HEIGHT, TileTextureId::Dirt)) {}