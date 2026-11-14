#include "config/farm_toml_reader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "config/toml_helpers.h"
#include "entities/collision_box.h"
#include "entities/collision_bounds.h"
#include "entities/entity_dimensions.h"
#include "farm/animals/animal_species.h"
#include "farm/decorations/decoration.h"

namespace {

using toml_helpers::invalid;
using toml_helpers::integer;
using toml_helpers::optionalBool;
using toml_helpers::string;
using toml_helpers::tableAt;

constexpr int TILE_SIZE = entity_dimensions::TILE_SIZE;
constexpr int MAX_FARM_DIMENSION =
    std::numeric_limits<uint16_t>::max() / TILE_SIZE;
constexpr std::size_t MAX_FARM_TILES = 1024 * 1024;

struct BackgroundAppearance {
    uint16_t textureId;
    bool walkable;
};

bool readWalkableState(const toml::table& table,
                       const std::string& field) {
    const auto state = string(table, "state", field + ".state");
    if (state != "walkable" && state != "unwalkable") {
        invalid(field + ".state", "se esperaba walkable o unwalkable");
    }
    return state == "walkable";
}

BackgroundAppearance readBackgroundAppearance(
    const toml::table& table, const std::string& field) {
    const auto texture = static_cast<uint16_t>(integer(
        table, "texture_id", std::numeric_limits<uint16_t>::max(),
        field + ".texture_id"));

    return {texture, readWalkableState(table, field)};
}

template <class Callback>
void farmEntries(const toml::table& table,
                 std::string_view key,
                 Callback callback,
                 bool required = false,
                 std::size_t limit =
                     std::numeric_limits<uint16_t>::max()) {
    const auto field = "farm." + std::string(key);
    const auto* node = table.get(key);

    if (!node && !required) {
        return;
    }
    if (!node || !node->is_array()) {
        invalid(field, "se esperaba una lista");
    }

    const auto& array = *node->as_array();
    if (array.size() > limit) {
        invalid(field, "demasiados elementos");
    }

    for (std::size_t i = 0; i < array.size(); ++i) {
        const auto item = field + "[" + std::to_string(i) + "]";
        callback(tableAt(&array[i], item), item);
    }
}

Position farmTileAt(const toml::table& table,
                    int width,
                    int height,
                    const std::string& field) {
    return Position{
        static_cast<int>(integer(table, "x", width - 1, field + ".x")),
        static_cast<int>(integer(table, "y", height - 1, field + ".y"))
    };
}

Position farmBaseAt(const toml::node* node,
                    const ZoneMap& terrain,
                    int width,
                    int height,
                    const std::string& field) {
    const auto tile = farmTileAt(tableAt(node, field), width, height, field);
    const auto base = terrain.baseAt({tile.getY(), tile.getX()});

    if (!base || !terrain.isWalkableBase(*base)) {
        invalid(field, "el tile es void o unwalkable");
    }

    return *base;
}

template <class Type, std::size_t N>
Type farmNamedType(
    const toml::table& table,
    const std::pair<std::string_view, Type> (&types)[N],
    const std::string& field) {
    const auto name = string(table, "type", field + ".type");

    for (const auto& [label, type] : types) {
        if (label == name) {
            return type;
        }
    }

    invalid(field + ".type", "tipo desconocido: " + name);
}

void reserveFarmPosition(FarmLayout& layout, const Position& base) {
    const auto alreadyReserved = std::any_of(
        layout.reservedPositions.begin(), layout.reservedPositions.end(),
        [&](const auto& other) {
            return layout.terrain.sameBaseCell(base, other);
        });

    if (!alreadyReserved) {
        layout.reservedPositions.push_back(base);
    }
}

void readBackground(const toml::table& farm,
                    int width,
                    int height,
                    FarmLayout& layout,
                    std::vector<std::vector<bool>>& walkable) {
    const bool hasDefault = farm.contains("background_default");
    const bool hasOverrides = farm.contains("background_overrides");
    const bool hasLegacy = farm.contains("background");

    if (hasLegacy && (hasDefault || hasOverrides)) {
        invalid("farm.background",
                "no se puede combinar el formato anterior con default/overrides");
    }

    if (hasDefault) {
        const auto& defaultTable = tableAt(
            farm.get("background_default"), "farm.background_default");
        const auto defaultAppearance = readBackgroundAppearance(
            defaultTable, "farm.background_default");

        std::vector<std::vector<BackgroundAppearance>> appearances(
            height, std::vector<BackgroundAppearance>(width, defaultAppearance));
        std::vector<std::vector<bool>> overridden(
            height, std::vector<bool>(width, false));

        farmEntries(farm, "background_overrides",
            [&](const auto& entry, const auto& field) {
                const auto tile = farmTileAt(entry, width, height, field);
                const auto x = tile.getX();
                const auto y = tile.getY();

                if (overridden[y][x]) {
                    invalid(field, "override de tile repetido");
                }
                overridden[y][x] = true;

                const auto texture = static_cast<uint16_t>(integer(
                    entry, "texture_id", std::numeric_limits<uint16_t>::max(),
                    field + ".texture_id"));
                bool tileWalkable = defaultAppearance.walkable;
                if (const auto* stateNode = entry.get("state")) {
                    if (!stateNode->is_string()) {
                        invalid(field + ".state", "se esperaba un texto");
                    }
                    const auto state = *stateNode->template value<std::string>();
                    if (state != "walkable" && state != "unwalkable") {
                        invalid(field + ".state",
                                "se esperaba walkable o unwalkable");
                    }
                    tileWalkable = state == "walkable";
                }

                appearances[y][x] = {texture, tileWalkable};
            }, false, MAX_FARM_TILES);

        layout.background.reserve(static_cast<std::size_t>(width) * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const auto appearance = appearances[y][x];
                walkable[y][x] = appearance.walkable;
                layout.background.push_back({
                    Position{x, y}, appearance.textureId
                });
            }
        }
        return;
    }

    if (hasOverrides) {
        invalid("farm.background_overrides",
                "requiere background_default");
    }

    std::vector<std::vector<bool>> defined(
        height, std::vector<bool>(width, false));

    farmEntries(farm, "background",
        [&](const auto& entry, const auto& field) {
            const auto tile = farmTileAt(entry, width, height, field);
            const int x = tile.getX();
            const int y = tile.getY();

            if (defined[y][x]) {
                invalid(field, "tile repetido");
            }
            defined[y][x] = true;

            const auto appearance = readBackgroundAppearance(entry, field);
            walkable[y][x] = appearance.walkable;
            layout.background.push_back({tile, appearance.textureId});
        }, true, MAX_FARM_TILES);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!defined[y][x]) {
                invalid("farm.background", "faltan tiles por definir");
            }
        }
    }
}

bool validBase(const Position& base) {
    return base.getX() >= 0 && base.getY() > 0 &&
           base.getX() <= std::numeric_limits<uint16_t>::max() &&
           base.getY() <= std::numeric_limits<uint16_t>::max();
}

bool walkableBounds(const ZoneMap& terrain, const CollisionBounds& bounds) {
    return bounds.isValid() && bounds.getLeft() >= 0 && bounds.getTop() >= 0 &&
           terrain.isWalkable(bounds);
}

bool intersectsAny(const CollisionBounds& bounds,
                   const std::vector<CollisionBounds>& others) {
    return std::any_of(others.begin(), others.end(),
        [&](const auto& other) { return bounds.intersects(other); });
}

const CropDefinition* findCropDefinition(const FarmConfig& config,
                                         CropType type) {
    const auto found = std::find_if(
        config.crops.begin(), config.crops.end(),
        [type](const auto& definition) { return definition.type == type; });
    return found == config.crops.end() ? nullptr : &*found;
}

const AnimalDefinition* findAnimalDefinition(const FarmConfig& config,
                                             AnimalType type) {
    const auto species = animalSpeciesForType(type);
    if (!species) {
        return nullptr;
    }

    const auto found = std::find_if(
        config.animals.begin(), config.animals.end(),
        [&](const auto& definition) { return definition.species == *species; });
    return found == config.animals.end() ? nullptr : &*found;
}

void validateInitialPlacements(const FarmLayout& layout,
                               const FarmConfig& config) {
    config.validate();

    if (!validBase(layout.spawn)) {
        invalid("farm.spawn", "posicion base invalida");
    }
    for (const auto& base : layout.terrain.walkableBasePositions()) {
        if (!validBase(base)) {
            invalid("farm", "el mapa contiene una posicion base fuera de rango");
        }
    }

    const CollisionBox cropBox{entity_dimensions::CROP};
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    std::vector<CollisionBounds> reservedBounds;
    reservedBounds.reserve(layout.reservedPositions.size());

    for (const auto& base : layout.reservedPositions) {
        if (!validBase(base)) {
            invalid("farm.reserved_positions", "posicion base invalida");
        }
        const auto bounds = cropBox.at(base);
        if (!walkableBounds(layout.terrain, bounds)) {
            invalid("farm.reserved_positions",
                    "una posicion reservada no tiene espacio caminable");
        }
        reservedBounds.push_back(bounds);
    }

    std::set<ZoneMap::Cell> tilledCells;
    for (const auto& base : layout.tilledSoil) {
        const auto cell = layout.terrain.baseCellAt(base);
        const auto bounds = cropBox.at(base);
        if (!validBase(base) || !cell ||
            !walkableBounds(layout.terrain, bounds) ||
            intersectsAny(bounds, reservedBounds)) {
            invalid("farm.tilled_soil", "posicion de suelo labrado invalida u ocupada");
        }
        if (!tilledCells.insert(*cell).second) {
            invalid("farm.tilled_soil", "tile repetido");
        }
    }

    std::vector<CollisionBounds> occupiedBounds;
    std::vector<bool> blocksMovement;
    std::map<AnimalSpecies, std::size_t> animalCounts;

    const auto canPlace = [&](const CollisionBounds& bounds) {
        return walkableBounds(layout.terrain, bounds) &&
               !intersectsAny(bounds, reservedBounds) &&
               !intersectsAny(bounds, occupiedBounds);
    };

    for (const auto& spawn : layout.animals) {
        const auto* definition = findAnimalDefinition(config, spawn.type);
        if (!validBase(spawn.base) || !definition) {
            invalid("farm.animals", "tipo o posicion de animal no soportado");
        }

        const auto bounds = definition->collision.at(spawn.base);
        auto species = animalSpeciesForType(spawn.type);
        if (!canPlace(bounds) || !species ||
            ++animalCounts[*species] > definition->maxCount) {
            invalid("farm.animals", "animal superpuesto, fuera del mapa o excede el limite");
        }

        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(true);
    }

    for (const auto& spawn : layout.crops) {
        if (!validBase(spawn.base) || !findCropDefinition(config, spawn.type)) {
            invalid("farm.crops", "tipo o posicion de cultivo no soportado");
        }

        const auto cell = layout.terrain.baseCellAt(spawn.base);
        const auto bounds = cropBox.at(spawn.base);
        if (!cell) {
            invalid("farm.crops", "posicion de cultivo fuera del mapa");
        }

        if (!tilledCells.contains(*cell)) {
            if (!canPlace(bounds)) {
                invalid("farm.crops", "no se puede labrar el tile del cultivo");
            }
            tilledCells.insert(*cell);
        }

        if (!canPlace(bounds)) {
            invalid("farm.crops", "cultivo superpuesto o fuera del mapa");
        }
        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(true);
    }

    const CollisionBox decorationBox{entity_dimensions::DECORATION};
    for (const auto& spawn : layout.decorations) {
        if (!validBase(spawn.base) || !Decoration::isDecoration(spawn.item)) {
            invalid("farm.furniture", "mueble o posicion no soportados");
        }

        const auto bounds = decorationBox.at(spawn.base);
        if (!canPlace(bounds)) {
            invalid("farm.furniture", "mueble superpuesto o fuera del mapa");
        }
        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(spawn.item != ItemId::Rug);
    }

    const auto playerBounds = playerBox.at(layout.spawn);
    if (!walkableBounds(layout.terrain, playerBounds)) {
        invalid("farm.spawn", "el jugador no tiene espacio caminable en el spawn");
    }

    for (std::size_t i = 0; i < occupiedBounds.size(); ++i) {
        if (blocksMovement[i] && playerBounds.intersects(occupiedBounds[i])) {
            invalid("farm.spawn", "el spawn del jugador esta ocupado por una entidad");
        }
    }
}

FarmLayout readFarmLayout(const toml::table& root,
                          const FarmConfig& config) {
    const auto& farm = tableAt(root.get("farm"), "farm");

    const int width = static_cast<int>(integer(
        farm, "width", MAX_FARM_DIMENSION, "farm.width"));
    const int height = static_cast<int>(integer(
        farm, "height", MAX_FARM_DIMENSION, "farm.height"));

    if (width == 0 || height == 0 ||
        static_cast<std::size_t>(width) * height > MAX_FARM_TILES) {
        invalid("farm", "dimensiones vacias o demasiado grandes");
    }

    FarmLayout layout;
    std::vector<std::vector<bool>> walkable(
        height, std::vector<bool>(width, false));
    readBackground(farm, width, height, layout, walkable);
    layout.terrain = ZoneMap{walkable, TILE_SIZE};

    layout.spawn = farmBaseAt(
        farm.get("spawn"), layout.terrain, width, height, "farm.spawn");
    reserveFarmPosition(layout, layout.spawn);

    if (farm.contains("player_houses")) {
        const int count = static_cast<int>(integer(
            farm, "player_houses", 4, "farm.player_houses"));
        if (count == 0) {
            invalid("farm.player_houses", "debe haber entre 1 y 4 casas");
        }
        layout.playerHouses = static_cast<uint8_t>(count);
    }

    if (farm.contains("door_mines")) {
        layout.mineDoor = farmBaseAt(
            farm.get("door_mines"), layout.terrain, width, height,
            "farm.door_mines");
        layout.mineReturn = farm.contains("mine_return")
            ? farmBaseAt(farm.get("mine_return"), layout.terrain, width,
                         height, "farm.mine_return")
            : layout.spawn;

        reserveFarmPosition(layout, *layout.mineDoor);
        reserveFarmPosition(layout, *layout.mineReturn);
    } else if (farm.contains("mine_return")) {
        invalid("farm.mine_return", "requiere door_mines");
    }

    if (farm.contains("door_town")) {
        layout.townDoor = farmBaseAt(farm.get("door_town"), layout.terrain,width, height, "farm.door_town");
        layout.townReturn = farm.contains("town_return")
            ? farmBaseAt(farm.get("town_return"), layout.terrain, width,
                         height, "farm.town_return")
            : layout.spawn;
        if (layout.mineDoor && layout.terrain.sameBaseCell(*layout.mineDoor, *layout.townDoor)) {
            invalid("farm.door_town", "no puede coincidir con door_mines");
        }
        reserveFarmPosition(layout, *layout.townDoor);
        reserveFarmPosition(layout, *layout.townReturn);
    } else if (farm.contains("town_return")) {
        invalid("farm.town_return", "requiere door_town");
    }

    farmEntries(farm, "beds", [&](const auto& entry, const auto& field) {
        const auto base = farmBaseAt(&entry, layout.terrain, width, height, field);
        const auto conflicts = [&](const Position& other) {
            return layout.terrain.sameBaseCell(base, other);
        };
        if (std::any_of(layout.beds.begin(), layout.beds.end(), conflicts) ||
            (layout.mineDoor && conflicts(*layout.mineDoor)) ||
            (layout.townDoor && conflicts(*layout.townDoor))) {
            invalid(field, "la cama coincide con otra cama o una puerta");
        }
        layout.beds.push_back(base);
        reserveFarmPosition(layout, base);
    }, false, 4);

    farmEntries(farm, "reserved_positions",
        [&](const auto& entry, const auto& field) {
            reserveFarmPosition(
                layout,
                farmBaseAt(&entry, layout.terrain, width, height, field));
        });

    farmEntries(farm, "tilled_soil",
        [&](const auto& entry, const auto& field) {
            layout.tilledSoil.push_back(
                farmBaseAt(&entry, layout.terrain, width, height, field));
        });

    constexpr std::pair<std::string_view, CropType> cropTypes[]{
        {"strawberry", CropType::Strawberry},
        {"scallion", CropType::Scallion},
        {"potato", CropType::Potato},
        {"pumpkin", CropType::Pumpkin}
    };

    farmEntries(farm, "crops",
        [&](const auto& entry, const auto& field) {
            if (entry.contains("growth_stage")) {
                invalid(field + ".growth_stage",
                        "la carga inicial crea cultivos nuevos; "
                        "no restaura crecimiento");
            }

            layout.crops.push_back({
                farmBaseAt(entry.get("position"), layout.terrain,
                           width, height, field + ".position"),
                farmNamedType(entry, cropTypes, field),
                optionalBool(entry, "watered", field + ".watered")
            });
        });

    constexpr std::pair<std::string_view, AnimalType> animalTypes[]{
        {"brown_cow", AnimalType::BrownCow},
        {"white_cow", AnimalType::WhiteCow},
        {"brown_chicken", AnimalType::BrownChicken},
        {"white_chicken", AnimalType::WhiteChicken},
        {"blue_chicken", AnimalType::BlueChicken},
        {"void_chicken", AnimalType::VoidChicken}
    };

    farmEntries(farm, "animals",
        [&](const auto& entry, const auto& field) {
            layout.animals.push_back({
                farmBaseAt(entry.get("position"), layout.terrain,
                           width, height, field + ".position"),
                farmNamedType(entry, animalTypes, field)
            });
        });

    farmEntries(farm, "furniture",
        [&](const auto& entry, const auto& field) {
            const auto item = static_cast<ItemId>(integer(
                entry, "item_id", std::numeric_limits<uint16_t>::max(),
                field + ".item_id"));

            if (!Decoration::isDecoration(item)) {
                invalid(field + ".item_id", "mueble no soportado");
            }

            layout.decorations.push_back({
                farmBaseAt(entry.get("position"), layout.terrain,
                           width, height, field + ".position"),
                item
            });
        });

    farmEntries(farm, "chests",
        [](const auto&, const auto& field) {
            invalid(field,
                    "la carga de cofres con inventario "
                    "todavia no esta implementada");
        });

    for (const auto& base : layout.reservedPositions) {
        if (!layout.terrain.hasBasePath(layout.spawn, base)) {
            invalid("farm", "spawn y accesos reservados no estan conectados");
        }
    }

    validateInitialPlacements(layout, config);
    return layout;
}

}  // namespace

FarmTomlReader::FarmTomlReader(const FarmConfig& config) : config(config) {}

FarmLayout FarmTomlReader::read(const toml::table& root) const {
    return readFarmLayout(root, config);
}
