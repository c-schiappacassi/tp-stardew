#include "player_factory.h"

#include <initializer_list>
#include <random>
#include "constants.h"
#include "entities/entity_dimensions.h"

PlayerFactory::PlayerFactory(const PlayerConfig& config):config(config) {}

Player PlayerFactory::create(const PlayerData& appearance) const {
    PlayerData spawn = appearance;

    const int tileSize = entity_dimensions::TILE_SIZE;
    spawn.pos = Position{3 * tileSize + tileSize / 2, 4 * tileSize};
    spawn.currentZone = Zone::Farm;

    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<uint32_t> goldDistribution(0, 100);

    Player player(spawn,PLAYER_INVENTORY_CAPACITY,PLAYER_INITIAL_HEALTH,PLAYER_INITIAL_ENERGY,goldDistribution(generator),config);

    for (const ItemId tool : {ItemId::Pickaxe, ItemId::WateringCan,ItemId::Shovel, ItemId::Sword}) {
        player.addItem(Stack{tool, 1, Quality::None});
    }

    return player;
}
