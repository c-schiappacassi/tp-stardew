#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "entities/entity_dimensions.h"
#include "farm/farm_config.h"
#include "farm/loading/farm_layout.h"

#include <toml++/toml.hpp>

class CollisionBounds;

class FarmTomlReader {
private:
    static constexpr int TILE_SIZE = entity_dimensions::TILE_SIZE;
    static constexpr int MAX_FARM_DIMENSION =
        std::numeric_limits<uint16_t>::max() / TILE_SIZE;
    static constexpr std::size_t MAX_FARM_TILES = 1024 * 1024;

    struct BackgroundAppearance {
        uint16_t textureId;
        bool walkable;
    };

    const FarmConfig& config;

    static bool readWalkableState(const toml::table& table,
                                  const std::string& field);
    static BackgroundAppearance readBackgroundAppearance(
        const toml::table& table, const std::string& field);

    template <class Callback>
    static void farmEntries(const toml::table& table,
                            std::string_view key,
                            Callback callback,
                            bool required = false,
                            std::size_t limit =
                                std::numeric_limits<uint16_t>::max());

    static Position farmTileAt(const toml::table& table,
                               int width,
                               int height,
                               const std::string& field);
    static Position farmBaseAt(const toml::node* node,
                               const ZoneMap& terrain,
                               int width,
                               int height,
                               const std::string& field);

    template <class Type, std::size_t N>
    static Type farmNamedType(
        const toml::table& table,
        const std::pair<std::string_view, Type> (&types)[N],
        const std::string& field);

    static std::map<uint8_t, Stack> readChestItems(const toml::table& entry,
                                                   const std::string& field);
    static void reserveFarmPosition(FarmLayout& layout, const Position& base);
    static void readBackground(const toml::table& farm,
                               int width,
                               int height,
                               FarmLayout& layout,
                               std::vector<std::vector<bool>>& walkable);
    static bool validBase(const Position& base);
    static bool walkableBounds(const ZoneMap& terrain,
                               const CollisionBounds& bounds);
    static bool intersectsAny(const CollisionBounds& bounds,
                              const std::vector<CollisionBounds>& others);
    static const CropDefinition* findCropDefinition(const FarmConfig& config,
                                                    CropType type);
    static const AnimalDefinition* findAnimalDefinition(const FarmConfig& config,
                                                        AnimalType type);
    static void validateInitialPlacements(const FarmLayout& layout,
                                          const FarmConfig& config);
    static FarmLayout readFarmLayout(const toml::table& root,
                                     const FarmConfig& config);

public:
    explicit FarmTomlReader(const FarmConfig& config);

    FarmLayout read(const toml::table& root) const;
};
