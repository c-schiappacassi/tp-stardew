#pragma once

#include <cstdint>
#include <string>

#include "protocol_types.h"

enum class CheatAction {
    SetLife,
    SetEnergy,
    AddGold,
    RemoveGold,
    CompleteBundle,
    CompleteAllBundles,
    NextLevel,
    CareCrops,
    CareAnimals,
    Heal,
    GiveItem,
    ClearSlot,
    SpawnAnimal,
    Warp,
    EndDay,
    SpawnEnemy,
    KillEnemy,
    KillEnemies,
    ListItems
};

struct CheatCommand {
    CheatAction action;
    uint32_t amount = 0;
    std::string target;
    Quality quality = Quality::None;
};
