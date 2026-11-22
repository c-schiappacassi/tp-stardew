#pragma once

#include <cstdint>
#include <string>

enum class CheatAction {
    SetLife,
    SetEnergy,
    AddGold,
    RemoveGold,
    CompleteBundle,
    CompleteAllBundles,
    NextLevel,
    CareCrops,
    CareAnimals
};

struct CheatCommand {
    CheatAction action;
    uint32_t amount = 0;
    std::string bundle;
};
