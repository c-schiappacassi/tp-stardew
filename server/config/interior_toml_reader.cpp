#include "interior_toml_reader.h"

#include "server_constants.h"

#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "commerce/shop_products.h"
#include "config/toml_helpers.h"
#include "world/interiors/interior.h"
#include "world/interiors/interior_furniture_position.h"

using toml_helpers::invalid;

const toml::array& InteriorTomlReader::arrayAt(const toml::node* node, std::string_view field) {
    if (!node || !node->is_array())
        invalid(std::string(field), server_constants::EXPECTED_AN_ARRAY);
    return *node->as_array();
}

const toml::array& InteriorTomlReader::tupleAt(const toml::node* node, std::size_t size, std::string_view field) {
    const auto& array = arrayAt(node, field);
    if (array.size() != size)
        invalid(std::string(field), server_constants::UNEXPECTED_TUPLE_SIZE);
    return array;
}

int InteriorTomlReader::numberAt(const toml::node* node, int maximum, const std::string& field) {
    if (!node || !node->is_integer())
        invalid(field, server_constants::EXPECTED_AN_INTEGER);
    const auto value = *node->value<int64_t>();
    if (value < 0 || value > maximum)
        invalid(field, server_constants::INTEGER_OUT_OF_RANGE);
    return static_cast<int>(value);
}

Position InteriorTomlReader::baseAt(const toml::array& tuple, const ZoneMap& terrain, int width, int height, const std::string& field) {
    const int x = numberAt(tuple.get(0), width - 1, field);
    const int y = numberAt(tuple.get(1), height - 1, field);
    return *terrain.baseAt({y, x});
}

Position InteriorTomlReader::positionAt(const toml::node* node, const ZoneMap& terrain, int width, int height, const std::string& field) {
    return baseAt(tupleAt(node, 2, field), terrain, width, height, field);
}

InteriorTemplate InteriorTomlReader::readTemplate(const toml::table& table, InteriorId id, const std::string& field) {
    const int width = numberAt(table.get(server_constants::WIDTH), 256, field + server_constants::FIELD_WIDTH);
    const int height = numberAt(table.get(server_constants::HEIGHT), 256, field + server_constants::FIELD_HEIGHT);
    if (width == 0 || height == 0 || width * height > std::numeric_limits<uint16_t>::max())
        invalid(field, server_constants::INVALID_INTERIOR_DIMENSIONS);

    InteriorTemplate result;
    std::vector<std::vector<bool>> floor(height, std::vector<bool>(width, false));
    std::vector<std::vector<bool>> seen(height, std::vector<bool>(width, false));
    const auto& background = arrayAt(table.get(server_constants::BACKGROUND), field + server_constants::FIELD_BACKGROUND);
    if (background.size() != static_cast<std::size_t>(width * height))
        invalid(field + server_constants::FIELD_BACKGROUND, server_constants::EVERY_FLOOR_TILE_MUST_BE_CONFIGURED_ONCE);
    for (const auto& node: background) {
        const auto& tile = tupleAt(&node, 4, field + server_constants::FIELD_BACKGROUND);
        const int x = numberAt(tile.get(0), width - 1, field + server_constants::FIELD_BACKGROUND_X);
        const int y = numberAt(tile.get(1), height - 1, field + server_constants::FIELD_BACKGROUND_Y);
        numberAt(tile.get(2), static_cast<int>(TileTextureId::DungeonInnerCornerTopLeft),
            field + server_constants::FIELD_BACKGROUND_TEXTURE_ID);
        const auto* state = tile.get(3)->as_string();
        if (seen[y][x] || !state || (state->get() != server_constants::WALKABLE && state->get() != server_constants::UNWALKABLE))
            invalid(field + server_constants::FIELD_BACKGROUND, server_constants::INVALID_OR_DUPLICATE_FLOOR_TILE);
        seen[y][x] = true;
        floor[y][x] = state->get() == server_constants::WALKABLE;
    }
    result.terrain = ZoneMap{floor, entity_dimensions::TILE_SIZE};
    result.entry = positionAt(table.get(server_constants::ENTRY), result.terrain, width, height, field + server_constants::FIELD_ENTRY);
    result.exit = positionAt(table.get(server_constants::EXIT), result.terrain, width, height, field + server_constants::FIELD_EXIT);
    result.background.interior = InteriorBackgroundData{Position{0, 0}, id, std::nullopt};

    if (const auto* beds = table.get(server_constants::BEDS)) {
        const auto& entries = arrayAt(beds, field + server_constants::FIELD_BEDS);
        if (id != InteriorId::PlayerHouse || entries.size() > 4)
            invalid(field + server_constants::FIELD_BEDS, server_constants::ONLY_PLAYER_HOUSES_CAN_CONTAIN_UP_TO_4_BEDS);
        for (const auto& bed: entries)
            result.beds.push_back(positionAt(&bed, result.terrain, width, height, field + server_constants::FIELD_BEDS));
    }
    const auto merchant = shop_products::merchantForInterior(id);
    if (const auto* npc = table.get(server_constants::NPC)) {
        if (!merchant)
            invalid(field + server_constants::FIELD_NPC, server_constants::THE_INTERIOR_DOES_NOT_BELONG_TO_A_SHOP);
        result.background.interior->npc = NpcData{
            positionAt(npc, result.terrain, width, height, field + server_constants::FIELD_NPC), *merchant};
    } else if (merchant) {
        invalid(field + server_constants::FIELD_NPC, server_constants::SHOP_INTERIORS_REQUIRE_A_MERCHANT_POSITION);
    }

    if (const auto* furniture = table.get(server_constants::FURNITURE)) {
        const auto& entries = arrayAt(furniture, field + server_constants::FIELD_FURNITURE);
        if (entries.size() > std::numeric_limits<uint16_t>::max() - result.beds.size() * 2)
            invalid(field + server_constants::FIELD_FURNITURE, server_constants::TOO_MANY_FURNITURE_ENTRIES);
        const int tileSize = result.terrain.getCellSize();
        for (const auto& node: entries) {
            const auto& item = tupleAt(&node, 4, field + server_constants::FIELD_FURNITURE);
            const int x = numberAt(item.get(0), width - 1, field + server_constants::FIELD_FURNITURE);
            const int y = numberAt(item.get(1), height - 1, field + server_constants::FIELD_FURNITURE);
            const auto type = static_cast<FurnitureId>(numberAt(item.get(2), static_cast<int>(FurnitureId::RobinPlantTable), field + server_constants::FIELD_FURNITURE_ID));
            const auto direction = static_cast<Direction>(numberAt(item.get(3), static_cast<int>(Direction::Right), field + server_constants::FIELD_FURNITURE_DIRECTION));
            const auto position = interiorFurnitureBaseFromTileOrigin(Position{x * tileSize, y * tileSize}, type, direction);
            result.background.furniture.push_back({position, type, direction});
        }
    }

    InteriorLayout layout;
    layout.location = {id, 0};
    result.applyTo(layout);
    if (!Interior::validLayout(layout) || !Interior(layout).hasAccessibleBeds())
        invalid(field, server_constants::INVALID_INTERIOR_PASSAGES_BEDS_OR_MERCHANT_POSITION);
    return result;
}

InteriorTemplates InteriorTomlReader::read(const toml::table& root) const {
    InteriorTemplates result;
    for (const auto& node: arrayAt(root.get(server_constants::INTERIORS), server_constants::INTERIORS)) {
        const auto& table = toml_helpers::tableAt(&node, server_constants::INTERIORS);
        const auto id = static_cast<InteriorId>(numberAt(table.get(server_constants::INTERIOR_ID),
            static_cast<int>(InteriorId::ComunityCenter), server_constants::INTERIORS_INTERIOR_ID));
        if (!result.emplace(id, readTemplate(table, id, server_constants::INTERIORS)).second)
            invalid(server_constants::INTERIORS_INTERIOR_ID, server_constants::DUPLICATE_INTERIOR_TEMPLATE);
    }
    return result;
}

InteriorTemplates InteriorTomlReader::readFile(const std::string& path) const {
    return read(toml::parse_file(path));
}
