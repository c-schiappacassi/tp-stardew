#include "config/farm_toml_reader.h"

#include "server_constants.h"

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

#include "commerce/shop_products.h"
#include "config/toml_helpers.h"
#include "config/town_toml_reader.h"
#include "entities/collision_bounds.h"
#include "entities/collision_box.h"
#include "entities/entity_dimensions.h"
#include "farm/animals/animal_species.h"
#include "farm/decorations/decoration.h"
#include "farm/decorations/decoration_items.h"
#include "inventory/inventory.h"
#include "town/town_map.h"

using toml_helpers::invalid;
using toml_helpers::integer;
using toml_helpers::optionalBool;
using toml_helpers::string;
using toml_helpers::tableAt;

bool FarmTomlReader::readWalkableState(const toml::table& table,
                       const std::string& field) {
    const auto state = string(table, server_constants::STATE, field + server_constants::FIELD_STATE);
    if (state != server_constants::WALKABLE && state != server_constants::UNWALKABLE) {
        invalid(field + server_constants::FIELD_STATE, server_constants::EXPECTED_WALKABLE_OR_UNWALKABLE);
    }
    return state == server_constants::WALKABLE;
}

FarmTomlReader::BackgroundAppearance FarmTomlReader::readBackgroundAppearance(
    const toml::table& table, const std::string& field) {
    const auto texture = static_cast<uint16_t>(integer(
        table, server_constants::TEXTURE_ID, std::numeric_limits<uint16_t>::max(),
        field + server_constants::FIELD_TEXTURE_ID));

    return {texture, readWalkableState(table, field)};
}

template <class Callback>
void FarmTomlReader::farmEntries(const toml::table& table,
                 std::string_view key,
                 Callback callback,
                 bool required,
                 std::size_t limit) {
    const auto field = server_constants::FARM_2 + std::string(key);
    const auto* node = table.get(key);

    if (!node && !required) {
        return;
    }
    if (!node || !node->is_array()) {
        invalid(field, server_constants::EXPECTED_AN_ARRAY);
    }

    const auto& array = *node->as_array();
    if (array.size() > limit) {
        invalid(field, server_constants::TOO_MANY_ENTRIES);
    }

    for (std::size_t i = 0; i < array.size(); ++i) {
        const auto item = field + server_constants::OPEN_BRACKET + std::to_string(i) + server_constants::CLOSE_BRACKET;
        callback(tableAt(&array[i], item), item);
    }
}

Position FarmTomlReader::farmTileAt(const toml::table& table,
                    int width,
                    int height,
                    const std::string& field) {
    return Position{
        static_cast<int>(integer(table, server_constants::X, width - 1, field + server_constants::FIELD_X)),
        static_cast<int>(integer(table, server_constants::Y, height - 1, field + server_constants::FIELD_Y))
    };
}

Position FarmTomlReader::farmBaseAt(const toml::node* node,
                    const ZoneMap& terrain,
                    int width,
                    int height,
                    const std::string& field) {
    const auto tile = farmTileAt(tableAt(node, field), width, height, field);
    const auto base = terrain.baseAt({tile.getY(), tile.getX()});

    if (!base || !terrain.isWalkableBase(*base)) {
        invalid(field, server_constants::THE_TILE_IS_VOID_OR_UNWALKABLE);
    }

    return *base;
}

template <class Type, std::size_t N>
Type FarmTomlReader::farmNamedType(
    const toml::table& table,
    const std::pair<std::string_view, Type> (&types)[N],
    const std::string& field) {
    const auto name = string(table, server_constants::TYPE, field + server_constants::FIELD_TYPE);

    for (const auto& [label, type] : types) {
        if (label == name) {
            return type;
        }
    }

    invalid(field + server_constants::FIELD_TYPE, server_constants::UNKNOWN_TYPE_PREFIX + name);
}

std::map<uint8_t, Stack> FarmTomlReader::readChestItems(const toml::table& entry, const std::string& field) {
    std::map<uint8_t, Stack> contents;
    if (const auto* itemsNode = entry.get(server_constants::ITEMS)) {
        const auto* items = itemsNode->as_array();
        if (!items || items->size() > CHEST_INVENTORY_CAPACITY) {
            invalid(field + server_constants::FIELD_ITEMS, server_constants::EXPECTED_UP_TO_30_INVENTORY_SLOTS);
        }
        for (const auto& node: *items) {
            const auto itemField = field + server_constants::FIELD_ITEMS;
            const auto& item = tableAt(&node, itemField);
            const auto slot = static_cast<uint8_t>(integer(item, server_constants::SLOT, CHEST_INVENTORY_CAPACITY - 1, itemField + server_constants::FIELD_SLOT));
            Stack stack{static_cast<ItemId>(integer(item, server_constants::ITEM_ID, UINT16_MAX, itemField + server_constants::FIELD_ITEM_ID)),
                static_cast<uint16_t>(integer(item, server_constants::QUANTITY, INVENTORY_STACK_LIMIT, itemField + server_constants::FIELD_QUANTITY)),
                item.contains(server_constants::QUALITY) ? static_cast<Quality>(integer(item, server_constants::QUALITY, static_cast<uint8_t>(Quality::Perfect), itemField + server_constants::FIELD_QUALITY)) : Quality::None};
            if (!contents.emplace(slot, stack).second) {
                invalid(itemField + server_constants::FIELD_SLOT, server_constants::DUPLICATE_CHEST_SLOT);
            }
        }
    }
    try {
        Inventory{CHEST_INVENTORY_CAPACITY}.restoreSlots(contents);
    } catch (const std::invalid_argument&) {
        invalid(field + server_constants::FIELD_ITEMS, server_constants::INVALID_CHEST_CONTENTS);
    }
    return contents;
}

void FarmTomlReader::reserveFarmPosition(FarmLayout& layout, const Position& base) {
    const auto alreadyReserved = std::any_of(
        layout.reservedPositions.begin(), layout.reservedPositions.end(),
        [&](const auto& other) {
            return layout.terrain.sameBaseCell(base, other);
        });

    if (!alreadyReserved) {
        layout.reservedPositions.push_back(base);
    }
}

void FarmTomlReader::readBackground(const toml::table& farm,
                    int width,
                    int height,
                    FarmLayout& layout,
                    std::vector<std::vector<bool>>& walkable) {
    const bool hasDefault = farm.contains(server_constants::BACKGROUND_DEFAULT);
    const bool hasOverrides = farm.contains(server_constants::BACKGROUND_OVERRIDES);
    const bool hasLegacy = farm.contains(server_constants::BACKGROUND);

    if (hasLegacy && (hasDefault || hasOverrides)) {
        invalid(server_constants::FARM_BACKGROUND,
                server_constants::CANNOT_COMBINE_THE_PREVIOUS_FORMAT_WITH_DEFAULT_OVERRIDES);
    }

    if (hasDefault) {
        const auto& defaultTable = tableAt(
            farm.get(server_constants::BACKGROUND_DEFAULT), server_constants::FARM_BACKGROUND_DEFAULT);
        const auto defaultAppearance = readBackgroundAppearance(
            defaultTable, server_constants::FARM_BACKGROUND_DEFAULT);

        std::vector<std::vector<BackgroundAppearance>> appearances(
            height, std::vector<BackgroundAppearance>(width, defaultAppearance));
        std::vector<std::vector<bool>> overridden(
            height, std::vector<bool>(width, false));

        farmEntries(farm, server_constants::BACKGROUND_OVERRIDES,
            [&](const auto& entry, const auto& field) {
                const auto tile = farmTileAt(entry, width, height, field);
                const auto x = tile.getX();
                const auto y = tile.getY();

                if (overridden[y][x]) {
                    invalid(field, server_constants::DUPLICATE_TILE_OVERRIDE);
                }
                overridden[y][x] = true;

                const auto texture = static_cast<uint16_t>(integer(
                    entry, server_constants::TEXTURE_ID, std::numeric_limits<uint16_t>::max(),
                    field + server_constants::FIELD_TEXTURE_ID));
                bool tileWalkable = defaultAppearance.walkable;
                if (const auto* stateNode = entry.get(server_constants::STATE)) {
                    if (!stateNode->is_string()) {
                        invalid(field + server_constants::FIELD_STATE, server_constants::EXPECTED_A_STRING);
                    }
                    const auto state = *stateNode->template value<std::string>();
                    if (state != server_constants::WALKABLE && state != server_constants::UNWALKABLE) {
                        invalid(field + server_constants::FIELD_STATE,
                                server_constants::EXPECTED_WALKABLE_OR_UNWALKABLE);
                    }
                    tileWalkable = state == server_constants::WALKABLE;
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
        invalid(server_constants::FARM_BACKGROUND_OVERRIDES,
                server_constants::REQUIRES_BACKGROUND_DEFAULT);
    }

    std::vector<std::vector<bool>> defined(
        height, std::vector<bool>(width, false));

    farmEntries(farm, server_constants::BACKGROUND,
        [&](const auto& entry, const auto& field) {
            const auto tile = farmTileAt(entry, width, height, field);
            const int x = tile.getX();
            const int y = tile.getY();

            if (defined[y][x]) {
                invalid(field, server_constants::DUPLICATE_TILE);
            }
            defined[y][x] = true;

            const auto appearance = readBackgroundAppearance(entry, field);
            walkable[y][x] = appearance.walkable;
            layout.background.push_back({tile, appearance.textureId});
        }, true, MAX_FARM_TILES);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!defined[y][x]) {
                invalid(server_constants::FARM_BACKGROUND, server_constants::MISSING_TILE_DEFINITIONS);
            }
        }
    }
}

bool FarmTomlReader::validBase(const Position& base) {
    return base.getX() >= 0 && base.getY() > 0 &&
           base.getX() <= std::numeric_limits<uint16_t>::max() &&
           base.getY() <= std::numeric_limits<uint16_t>::max();
}

bool FarmTomlReader::walkableBounds(const ZoneMap& terrain, const CollisionBounds& bounds) {
    return bounds.isValid() && bounds.getLeft() >= 0 && bounds.getTop() >= 0 &&
           terrain.isWalkable(bounds);
}

bool FarmTomlReader::intersectsAny(const CollisionBounds& bounds,
                   const std::vector<CollisionBounds>& others) {
    return std::any_of(others.begin(), others.end(),
        [&](const auto& other) { return bounds.intersects(other); });
}

const CropDefinition* FarmTomlReader::findCropDefinition(const FarmConfig& config,
                                         CropType type) {
    const auto found = std::find_if(
        config.crops.begin(), config.crops.end(),
        [type](const auto& definition) { return definition.type == type; });
    return found == config.crops.end() ? nullptr : &*found;
}

const AnimalDefinition* FarmTomlReader::findAnimalDefinition(const FarmConfig& config,
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

void FarmTomlReader::validateInitialPlacements(const FarmLayout& layout,
                               const FarmConfig& config) {
    config.validate();

    if (!validBase(layout.spawn)) {
        invalid(server_constants::FARM_SPAWN, server_constants::INVALID_BASE_POSITION);
    }
    for (const auto& base : layout.terrain.walkableBasePositions()) {
        if (!validBase(base)) {
            invalid(server_constants::FARM, server_constants::THE_MAP_CONTAINS_AN_OUT_OF_RANGE_BASE_POSITION);
        }
    }

    const CollisionBox cropBox{entity_dimensions::CROP};
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    std::vector<CollisionBounds> reservedBounds;
    reservedBounds.reserve(layout.reservedPositions.size());

    for (const auto& base : layout.reservedPositions) {
        if (!validBase(base)) {
            invalid(server_constants::FARM_RESERVED_POSITIONS, server_constants::INVALID_BASE_POSITION);
        }
        const auto bounds = cropBox.at(base);
        if (!walkableBounds(layout.terrain, bounds)) {
            invalid(server_constants::FARM_RESERVED_POSITIONS,
                    server_constants::A_RESERVED_POSITION_HAS_NO_WALKABLE_SPACE);
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
            invalid(server_constants::FARM_TILLED_SOIL, server_constants::INVALID_OR_OCCUPIED_TILLED_SOIL_POSITION);
        }
        if (!tilledCells.insert(*cell).second) {
            invalid(server_constants::FARM_TILLED_SOIL, server_constants::DUPLICATE_TILE);
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
            invalid(server_constants::FARM_ANIMALS, server_constants::UNSUPPORTED_ANIMAL_TYPE_OR_POSITION);
        }

        const auto bounds = definition->collision.at(spawn.base);
        auto species = animalSpeciesForType(spawn.type);
        if (!canPlace(bounds) || !species ||
            ++animalCounts[*species] > definition->maxCount) {
            invalid(server_constants::FARM_ANIMALS, server_constants::ANIMAL_OVERLAPS_IS_OUTSIDE_THE_MAP_OR_EXCEEDS_THE_LIMIT);
        }

        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(true);
    }

    for (const auto& spawn : layout.crops) {
        if (!validBase(spawn.base) || !findCropDefinition(config, spawn.type)) {
            invalid(server_constants::FARM_CROPS, server_constants::UNSUPPORTED_CROP_TYPE_OR_POSITION);
        }

        const auto cell = layout.terrain.baseCellAt(spawn.base);
        const auto bounds = cropBox.at(spawn.base);
        if (!cell) {
            invalid(server_constants::FARM_CROPS, server_constants::CROP_POSITION_IS_OUTSIDE_THE_MAP);
        }

        if (!tilledCells.contains(*cell)) {
            if (!canPlace(bounds)) {
                invalid(server_constants::FARM_CROPS, server_constants::CANNOT_TILL_THE_CROP_TILE);
            }
            tilledCells.insert(*cell);
        }

        if (!canPlace(bounds)) {
            invalid(server_constants::FARM_CROPS, server_constants::CROP_OVERLAPS_OR_IS_OUTSIDE_THE_MAP);
        }
        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(true);
    }

    const CollisionBox decorationBox{entity_dimensions::DECORATION};
    for (const auto& spawn : layout.decorations) {
        if (!validBase(spawn.base) || !decoration_items::isDecoration(spawn.item)) {
            invalid(server_constants::FARM_FURNITURE, server_constants::UNSUPPORTED_FURNITURE_OR_POSITION);
        }

        const auto bounds = Decoration::collisionBoxFor(spawn.item, Direction::Down).at(spawn.base);
        if (!canPlace(bounds)) {
            invalid(server_constants::FARM_FURNITURE, server_constants::FURNITURE_OVERLAPS_OR_IS_OUTSIDE_THE_MAP);
        }
        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(spawn.item != ItemId::BeigeRug && spawn.item != ItemId::BlueRug);
    }

    for (const auto& spawn: layout.chests) {
        const auto bounds = decorationBox.at(spawn.base);
        if (!validBase(spawn.base) || !canPlace(bounds)) {
            invalid(server_constants::FARM_CHESTS, server_constants::CHEST_OVERLAPS_OR_IS_OUTSIDE_THE_MAP);
        }
        occupiedBounds.push_back(bounds);
        blocksMovement.push_back(true);
    }

    const auto playerBounds = playerBox.at(layout.spawn);
    if (!walkableBounds(layout.terrain, playerBounds)) {
        invalid(server_constants::FARM_SPAWN, server_constants::THE_PLAYER_HAS_NO_WALKABLE_SPACE_AT_SPAWN);
    }

    for (std::size_t i = 0; i < occupiedBounds.size(); ++i) {
        if (blocksMovement[i] && playerBounds.intersects(occupiedBounds[i])) {
            invalid(server_constants::FARM_SPAWN, server_constants::THE_PLAYER_SPAWN_IS_OCCUPIED_BY_AN_ENTITY);
        }
    }
}

FarmLayout FarmTomlReader::readFarmLayout(const toml::table& root,
                          const FarmConfig& config) {
    const auto& farm = tableAt(root.get(server_constants::FARM), server_constants::FARM);

    const int width = static_cast<int>(integer(
        farm, server_constants::WIDTH, MAX_FARM_DIMENSION, server_constants::FARM_WIDTH));
    const int height = static_cast<int>(integer(
        farm, server_constants::HEIGHT, MAX_FARM_DIMENSION, server_constants::FARM_HEIGHT));

    if (width == 0 || height == 0 ||
        static_cast<std::size_t>(width) * height > MAX_FARM_TILES) {
        invalid(server_constants::FARM, server_constants::EMPTY_OR_OVERSIZED_DIMENSIONS);
    }

    FarmLayout layout;
    std::vector<std::vector<bool>> walkable(
        height, std::vector<bool>(width, false));
    readBackground(farm, width, height, layout, walkable);
    layout.terrain = ZoneMap{walkable, TILE_SIZE};

    layout.spawn = farmBaseAt(
        farm.get(server_constants::SPAWN), layout.terrain, width, height, server_constants::FARM_SPAWN);
    reserveFarmPosition(layout, layout.spawn);

    if (farm.contains(server_constants::PLAYER_HOUSES)) {
        const int count = static_cast<int>(integer(
            farm, server_constants::PLAYER_HOUSES, 4, server_constants::FARM_PLAYER_HOUSES));
        if (count == 0) {
            invalid(server_constants::FARM_PLAYER_HOUSES, server_constants::THERE_MUST_BE_BETWEEN_1_AND_4_HOUSES);
        }
        layout.playerHouses = static_cast<uint8_t>(count);
    }

    if (farm.contains(server_constants::DOOR_MINES)) {
        layout.mineDoor = farmBaseAt(
            farm.get(server_constants::DOOR_MINES), layout.terrain, width, height,
            server_constants::FARM_DOOR_MINES);
        layout.mineReturn = farm.contains(server_constants::MINE_RETURN)
            ? farmBaseAt(farm.get(server_constants::MINE_RETURN), layout.terrain, width,
                         height, server_constants::FARM_MINE_RETURN)
            : layout.spawn;

        reserveFarmPosition(layout, *layout.mineDoor);
        reserveFarmPosition(layout, *layout.mineReturn);
    } else if (farm.contains(server_constants::MINE_RETURN)) {
        invalid(server_constants::FARM_MINE_RETURN, server_constants::REQUIRES_DOOR_MINES);
    }

    if (farm.contains(server_constants::DOOR_TOWN)) {
        layout.townDoor = farmBaseAt(farm.get(server_constants::DOOR_TOWN), layout.terrain,width, height, server_constants::FARM_DOOR_TOWN);
        layout.townReturn = farm.contains(server_constants::TOWN_RETURN)
            ? farmBaseAt(farm.get(server_constants::TOWN_RETURN), layout.terrain, width,
                         height, server_constants::FARM_TOWN_RETURN)
            : layout.spawn;
        if (layout.mineDoor && layout.terrain.sameBaseCell(*layout.mineDoor, *layout.townDoor)) {
            invalid(server_constants::FARM_DOOR_TOWN, server_constants::CANNOT_MATCH_DOOR_MINES);
        }
        reserveFarmPosition(layout, *layout.townDoor);
        reserveFarmPosition(layout, *layout.townReturn);
    } else if (farm.contains(server_constants::TOWN_RETURN)) {
        invalid(server_constants::FARM_TOWN_RETURN, server_constants::REQUIRES_DOOR_TOWN);
    }

    if (farm.contains(server_constants::BEDS)) {
        invalid(server_constants::FARM_BEDS, server_constants::FIXED_BEDS_MUST_BE_CONFIGURED_INSIDE_FARM_INTERIORS);
    }
    std::optional<TownMap> exteriorTown;
    farmEntries(farm, server_constants::INTERIORS, [&](const auto& entry, const auto& field) {
        InteriorLayout room;
        room.location.interior_id = static_cast<InteriorId>(integer(
            entry, server_constants::INTERIOR_ID, static_cast<int>(InteriorId::ComunityCenter), field + server_constants::FIELD_INTERIOR_ID));
        room.location.house_index = static_cast<uint8_t>(integer(
            entry, server_constants::HOUSE_INDEX, 3, field + server_constants::FIELD_HOUSE_INDEX));
        const ZoneMap* exterior = &layout.terrain;
        int exteriorWidth = width;
        int exteriorHeight = height;
        if (entry.contains(server_constants::OUTSIDE_ZONE)) {
            const auto zone = string(entry, server_constants::OUTSIDE_ZONE, field + server_constants::FIELD_OUTSIDE_ZONE);
            if (zone == server_constants::TOWN) {
                if (!exteriorTown) {
                    exteriorTown.emplace();
                    if (!exteriorTown->initialize(TownTomlReader{}.read(root))) {
                        invalid(field, server_constants::INVALID_OUTSIDE_TOWN);
                    }
                }
                room.outside.zone = Zone::Town;
                exterior = &exteriorTown->getMap();
                exteriorWidth = exteriorTown->getWidth();
                exteriorHeight = exteriorTown->getHeight();
            } else if (zone != server_constants::FARM) {
                invalid(field + server_constants::FIELD_OUTSIDE_ZONE, server_constants::EXPECTED_FARM_OR_TOWN);
            }
        }
        room.outside.position = farmBaseAt(entry.get(server_constants::DOOR), *exterior, exteriorWidth, exteriorHeight, field + server_constants::FIELD_DOOR);
        room.returnPosition = farmBaseAt(entry.get(server_constants::RETURN), *exterior, exteriorWidth, exteriorHeight, field + server_constants::FIELD_RETURN);
        const int roomWidth = static_cast<int>(integer(entry, server_constants::WIDTH, 256, field + server_constants::FIELD_WIDTH));
        const int roomHeight = static_cast<int>(integer(entry, server_constants::HEIGHT, 256, field + server_constants::FIELD_HEIGHT));
        if (roomWidth == 0 || roomHeight == 0) invalid(field, server_constants::THE_INTERIOR_CANNOT_BE_EMPTY);
        const auto* areas = entry.get(server_constants::WALKABLE_AREAS);
        std::vector<std::vector<bool>> floor(roomHeight, std::vector<bool>(roomWidth, !areas));
        if (areas) {
            const auto* array = areas->as_array();
            if (!array) invalid(field + server_constants::FIELD_WALKABLE_AREAS, server_constants::EXPECTED_AN_ARRAY_OF_RECTANGLES);
            for (const auto& area: *array) {
                const auto& rect = tableAt(&area, field + server_constants::FIELD_WALKABLE_AREAS);
                const int x = static_cast<int>(integer(rect, server_constants::X, roomWidth - 1, field + server_constants::FIELD_WALKABLE_AREAS_X));
                const int y = static_cast<int>(integer(rect, server_constants::Y, roomHeight - 1, field + server_constants::FIELD_WALKABLE_AREAS_Y));
                const int w = static_cast<int>(integer(rect, server_constants::WIDTH, roomWidth - x, field + server_constants::FIELD_WALKABLE_AREAS_WIDTH));
                const int h = static_cast<int>(integer(rect, server_constants::HEIGHT, roomHeight - y, field + server_constants::FIELD_WALKABLE_AREAS_HEIGHT));
                if (w == 0 || h == 0) invalid(field, server_constants::EMPTY_WALKABLE_AREA);
                for (int row = y; row < y + h; ++row) {
                    for (int col = x; col < x + w; ++col) floor[row][col] = true;
                }
            }
        }
        room.terrain = ZoneMap{floor, TILE_SIZE};
        room.entry = farmBaseAt(entry.get(server_constants::ENTRY), room.terrain, roomWidth, roomHeight, field + server_constants::FIELD_ENTRY);
        room.exit = farmBaseAt(entry.get(server_constants::EXIT), room.terrain, roomWidth, roomHeight, field + server_constants::FIELD_EXIT);
        const auto* beds = entry.get(server_constants::BEDS);
        if (beds) {
            const auto* array = beds->as_array();
            if (!array || array->size() > 4) invalid(field + server_constants::FIELD_BEDS, server_constants::EXPECTED_AN_ARRAY_OF_UP_TO_4_BEDS);
            for (const auto& bed: *array) {
                room.beds.push_back(farmBaseAt(&bed, room.terrain, roomWidth, roomHeight, field + server_constants::FIELD_BEDS));
            }
        }
        farmEntries(entry, server_constants::CHESTS, [&](const auto& chest, const auto& chestField) {
            if (room.location.interior_id != InteriorId::PlayerHouse)
                invalid(field + server_constants::FIELD_CHESTS, server_constants::ONLY_PLAYER_HOUSES_CAN_CONTAIN_CHESTS);
            room.chests.push_back({farmBaseAt(chest.get(server_constants::POSITION), room.terrain, roomWidth, roomHeight, field + server_constants::FIELD_CHESTS_POSITION), readChestItems(chest, chestField)});
        });
        room.background.interior = InteriorBackgroundData{
            Position{0, 0}, room.location.interior_id, std::nullopt};
        if (const auto* npc = entry.get(server_constants::NPC)) {
            const auto merchant = shop_products::merchantForInterior(room.location.interior_id);
            if (!merchant) invalid(field + server_constants::FIELD_NPC, server_constants::THE_INTERIOR_DOES_NOT_BELONG_TO_A_SHOP);
            room.background.interior->npc = NpcData{
                farmBaseAt(npc, room.terrain, roomWidth, roomHeight, field + server_constants::FIELD_NPC), *merchant};
        }
        const auto conflicts = [&](const Position& other) {
            return exterior->sameBaseCell(room.outside.position, other);
        };
        if ((room.outside.zone == Zone::Farm && layout.mineDoor && conflicts(*layout.mineDoor)) ||
            (room.outside.zone == Zone::Farm && layout.townDoor && conflicts(*layout.townDoor)) ||
            std::any_of(layout.interiors.begin(), layout.interiors.end(), [&](const auto& other) {
                return (room.outside.zone == other.outside.zone && conflicts(other.outside.position)) ||
                       other.location == room.location;
            })) invalid(field, server_constants::DUPLICATE_INTERIOR_OR_DOOR);
        if (room.outside.zone == Zone::Farm) {
            reserveFarmPosition(layout, room.outside.position);
            reserveFarmPosition(layout, room.returnPosition);
        }
        layout.interiors.push_back(std::move(room));
    }, false, 9);

    farmEntries(farm, server_constants::RESERVED_POSITIONS,
        [&](const auto& entry, const auto& field) {
            reserveFarmPosition(
                layout,
                farmBaseAt(&entry, layout.terrain, width, height, field));
        });

    farmEntries(farm, server_constants::TILLED_SOIL_2,
        [&](const auto& entry, const auto& field) {
            layout.tilledSoil.push_back(
                farmBaseAt(&entry, layout.terrain, width, height, field));
        });

    constexpr std::pair<std::string_view, CropType> cropTypes[]{
        {server_constants::STRAWBERRY, CropType::Strawberry},
        {server_constants::SCALLION, CropType::Scallion},
        {server_constants::POTATO, CropType::Potato},
        {server_constants::PUMPKIN, CropType::Pumpkin}
    };

    farmEntries(farm, server_constants::CROPS,
        [&](const auto& entry, const auto& field) {
            if (entry.contains(server_constants::GROWTH_STAGE)) {
                invalid(field + server_constants::FIELD_GROWTH_STAGE,
                        server_constants::INITIAL_LOADING_CREATES_NEW_CROPS_IT_DOES_NOT_RESTORE_GROWTH);
            }

            layout.crops.push_back({
                farmBaseAt(entry.get(server_constants::POSITION), layout.terrain,
                           width, height, field + server_constants::FIELD_POSITION),
                farmNamedType(entry, cropTypes, field),
                optionalBool(entry, server_constants::WATERED, field + server_constants::FIELD_WATERED)
            });
        });

    constexpr std::pair<std::string_view, AnimalType> animalTypes[]{
        {server_constants::BROWN_COW, AnimalType::BrownCow},
        {server_constants::WHITE_COW, AnimalType::WhiteCow},
        {server_constants::BROWN_CHICKEN, AnimalType::BrownChicken},
        {server_constants::WHITE_CHICKEN, AnimalType::WhiteChicken},
        {server_constants::BLUE_CHICKEN, AnimalType::BlueChicken},
        {server_constants::VOID_CHICKEN, AnimalType::VoidChicken}
    };

    farmEntries(farm, server_constants::ANIMALS,
        [&](const auto& entry, const auto& field) {
            layout.animals.push_back({
                farmBaseAt(entry.get(server_constants::POSITION), layout.terrain,
                           width, height, field + server_constants::FIELD_POSITION),
                farmNamedType(entry, animalTypes, field)
            });
        });

    farmEntries(farm, server_constants::FURNITURE,
        [&](const auto& entry, const auto& field) {
            const auto item = static_cast<ItemId>(integer(
                entry, server_constants::ITEM_ID, std::numeric_limits<uint16_t>::max(),
                field + server_constants::FIELD_ITEM_ID));

            if (!decoration_items::isDecoration(item)) {
                invalid(field + server_constants::FIELD_ITEM_ID, server_constants::UNSUPPORTED_FURNITURE);
            }

            layout.decorations.push_back({
                farmBaseAt(entry.get(server_constants::POSITION), layout.terrain,
                           width, height, field + server_constants::FIELD_POSITION),
                item
            });
        });

    farmEntries(farm, server_constants::CHESTS, [&](const auto& entry, const auto& field) {
        FarmChestSpawn spawn{farmBaseAt(entry.get(server_constants::POSITION), layout.terrain, width, height, field + server_constants::FIELD_POSITION), {}};
        spawn.items = readChestItems(entry, field);
        layout.chests.push_back(std::move(spawn));
    });

    for (const auto& base : layout.reservedPositions) {
        if (!layout.terrain.hasBasePath(layout.spawn, base)) {
            invalid(server_constants::FARM, server_constants::SPAWN_AND_RESERVED_ENTRANCES_ARE_NOT_CONNECTED);
        }
    }

    validateInitialPlacements(layout, config);
    return layout;
}

FarmTomlReader::FarmTomlReader(const FarmConfig& config) : config(config) {}

FarmLayout FarmTomlReader::read(const toml::table& root) const {
    return readFarmLayout(root, config);
}
