#include "map_validator.h"

namespace {

const int MIN_HOUSES = 1;
const int MAX_HOUSES = 4;

int count(const ZoneMap& zone, ElementId element) {
    int total = 0;
    for (const PlacedElement& placed: zone.getElements()) {
        if (placed.element == element) {
            ++total;
        }
    }
    return total;
}

int countBuildings(const ZoneMap& zone) {
    int total = 0;
    for (const PlacedElement& placed: zone.getElements()) {
        if (placed.element.kind == ElementKind::Building) {
            ++total;
        }
    }
    return total;
}

void requireOne(std::vector<std::string>& errors, const std::string& zoneName, const ZoneMap& zone, ElementId element) {
    const int total = count(zone, element);
    const std::string name = ElementCatalog::infoOf(element).name;
    if (total == 0) {
        errors.push_back(zoneName + ": missing " + name);
    } else if (total > 1) {
        errors.push_back(zoneName + ": only one " + name + " allowed (has " + std::to_string(total) + ")");
    }
}

}

std::vector<std::string> validateMap(const WorldMap& map) {
    std::vector<std::string> errors;
    //farm: 1 a 4 casas y la entrada a la dungeon
    const int houses = countBuildings(map.farm);
    if (houses < MIN_HOUSES || houses > MAX_HOUSES) {
        errors.push_back("Farm: needs between " + std::to_string(MIN_HOUSES) + " and " + std::to_string(MAX_HOUSES) +
                         " houses (has " + std::to_string(houses) + ")");
    }
    requireOne(errors, "Farm", map.farm, portalElement(PortalId::FarmToMine));

    //town: las casas de los npc y el comunity center
    for (BuildingId building: {BuildingId::PierresShop, BuildingId::ClintsShop, BuildingId::MarniesShop,
                               BuildingId::RobinsShop, BuildingId::ComunityCenter}) {
        requireOne(errors, "Town", map.town, buildingElement(building));
    }

    //dungeon levels: mine exit y ladder to next level
    for (std::size_t i = 0; i < map.mineLevels.size(); ++i) {
        const std::string level = "Level " + std::to_string(i + 1);
        requireOne(errors, level, map.mineLevels[i], portalElement(PortalId::MineToFarm));
        requireOne(errors, level, map.mineLevels[i], portalElement(PortalId::ToNextLevel));
    }
    return errors;
}
