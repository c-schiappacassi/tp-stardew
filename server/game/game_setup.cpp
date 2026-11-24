#include "game_setup.h"

#include <utility>
#include "dungeon/dungeon_config.h"
#include "entities/entity_dimensions.h"
#include "commerce/shop_products.h"

InteriorLayout makePlayerHouseInterior(uint8_t houseIndex) {
    InteriorLayout house;
    house.location = {InteriorId::PlayerHouse, houseIndex};
    std::vector<std::vector<bool>> floor(17, std::vector<bool>(17, false));
    for (int row = 4; row <= 13; ++row) {
        for (int col = 1; col <= 15; ++col) floor[row][col] = true;
    }
    for (int row = 14; row <= 16; ++row) floor[row][3] = true;
    house.terrain = ZoneMap{floor, entity_dimensions::TILE_SIZE};
    house.outside.zone = Zone::Farm;
    house.entry = house.terrain.baseAt({16, 3}).value();
    house.exit = house.terrain.baseAt({16, 3}).value();
    house.beds = {house.terrain.baseAt({6, 12}).value()};
    house.background.interior = InteriorBackgroundData{
        Position{0, 0}, InteriorId::PlayerHouse, std::nullopt};
    return house;
}

InteriorLayout makeTownInterior(InteriorId interiorId) {
    InteriorLayout interior;
    interior.location = {interiorId, 0};
    std::vector<std::vector<bool>> floor(17, std::vector<bool>(17, false));
    for (int row = 4; row <= 13; ++row) {
        for (int col = 1; col <= 15; ++col) floor[row][col] = true;
    }
    for (int row = 14; row <= 16; ++row) floor[row][3] = true;
    interior.terrain = ZoneMap{floor, entity_dimensions::TILE_SIZE};
    interior.outside.zone = Zone::Town;
    interior.entry = interior.terrain.baseAt({16, 3}).value();
    interior.exit = interior.terrain.baseAt({16, 3}).value();
    interior.background.interior = InteriorBackgroundData{
        Position{0, 0}, interiorId, std::nullopt};
    if (const auto npc = shop_products::merchantForInterior(interiorId)) {
        interior.background.interior->npc = NpcData{
            interior.terrain.baseAt({6, 8}).value(), *npc};
    }
    return interior;
}

GameSetup makeDefaultGameSetup(uint8_t worldId) {
    GameSetup setup;
    setup.worldId = worldId;
    const ZoneMap terrain(std::vector<std::vector<bool>>(24, std::vector<bool>(32, true)), entity_dimensions::TILE_SIZE);
    for (uint8_t level = 1; level <= DUNGEON_ROOM_COUNT; ++level) {
        DungeonRoomLayout room;
        room.terrain = terrain;
        room.passages.entry = terrain.baseAt({3, 3}).value();
        room.passages.exit = terrain.baseAt({3, 1}).value();
        room.passages.nextRoom = terrain.baseAt({21, 29}).value();
        setup.layout.emplace(level, std::move(room));
    }
    setup.entrance.emplace();
    setup.entrance->terrain = terrain;
    setup.entrance->position = terrain.baseAt({3, 3}).value();
    setup.entrance->returnPosition = terrain.baseAt({4, 3}).value();
    setup.farm.terrain = terrain;
    for (const auto& base : terrain.walkableBasePositions()) {
        const auto cell = terrain.baseCellAt(base).value();
        setup.farm.background.push_back({Position{cell.second, cell.first}, static_cast<uint16_t>(TileTextureId::Dirt)});
    }
    setup.farm.spawn = setup.entrance->returnPosition;
    setup.farm.reservedPositions = {setup.farm.spawn, setup.entrance->position};
    setup.farm.townDoor = terrain.baseAt({3, 26}).value();
    setup.farm.townReturn = terrain.baseAt({4, 26}).value();
    for (uint8_t index = 0; index < setup.farm.playerHouses; ++index) {
        InteriorLayout house = makePlayerHouseInterior(index);
        house.outside.zone = Zone::Farm;
        house.outside.position = terrain.baseAt({6, 6 + index * 5}).value();
        house.returnPosition = terrain.baseAt({7, 6 + index * 5}).value();
        setup.farm.interiors.push_back(std::move(house));
    }
    setup.townConfig.map = {32, 24, entity_dimensions::TILE_SIZE};
    setup.townConfig.connections = {{"farm", {2, 12}}, {"dungeon", {29, 12}}};
    struct TownBuilding {
        InteriorId interior;
        int doorColumn;
    };
    const TownBuilding buildings[]{
        {InteriorId::MarniesShop, 5}, {InteriorId::PierresShop, 8},
        {InteriorId::RobinsShop, 20}, {InteriorId::ClintsShop, 26},
        {InteriorId::ComunityCenter, 14}
    };
    for (const auto& building : buildings) {
        InteriorLayout interior = makeTownInterior(building.interior);
        interior.outside.position = terrain.baseAt({12, building.doorColumn}).value();
        interior.returnPosition = terrain.baseAt({13, building.doorColumn}).value();
        setup.farm.interiors.push_back(std::move(interior));
    }
    return setup;
}

GameSetup makeFarmGameSetup(uint8_t worldId, const FarmLayout& farm) {
    auto setup = makeDefaultGameSetup(worldId);
    setup.farm = farm;
    setup.townConfig = {};
    setup.entrance.reset();
    if (farm.mineDoor) {
        setup.entrance.emplace();
        setup.entrance->terrain = farm.terrain;
        setup.entrance->position = *farm.mineDoor;
        setup.entrance->returnPosition = farm.mineReturn.value_or(farm.spawn);
    }
    return setup;
}
