#include "config/game_toml_reader.h"

#include <sstream>
#include <stdexcept>

#include <toml++/toml.hpp>

#include "config/combat_toml_reader.h"
#include "config/community_toml_reader.h"
#include "config/death_toml_reader.h"
#include "config/dungeon_toml_reader.h"
#include "config/farm_toml_reader.h"
#include "config/shop_toml_reader.h"
#include "config/town_toml_reader.h"

WorldData GameTomlReader::read(const std::string& path) const {
    try {
        const auto root = toml::parse_file(path);

        WorldData world;
        world.config.death = DeathTomlReader{}.read(root);
        world.config.market = ShopTomlReader{}.readMarket(root);
        world.config.community = CommunityTomlReader{}.read(root);
        world.config.robin = world.config.market.robin;
        world.config.dungeon = DungeonTomlReader{}.read(root);
        CombatTomlReader{}.read(root, world.config.dungeon, world.config.player);
        world.config.farm = FarmConfig{std::chrono::milliseconds{world.config.player.dayDurationMs}};

        if (root.contains("town")) {
            world.config.town = TownTomlReader{}.read(root);
        }

        if (root.contains("farm")) {
            world.farm = FarmTomlReader{world.config.farm}.read(root);
        }

        return world;

    } catch (const toml::parse_error& error) {
        std::ostringstream message;
        message << path << ": " << error;
        throw std::invalid_argument(message.str());

    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(path + ": " + error.what());
    }
}
