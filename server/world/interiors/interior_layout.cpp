#include "interior_layout.h"

#include "commerce/shop_products.h"
#include "entities/entity_dimensions.h"

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
    house.beds = {house.terrain.baseAt({5, 12}).value()};
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

