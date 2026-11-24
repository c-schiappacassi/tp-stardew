#pragma once

#include "world/contracts/world_setup.h"

using GameSetup = WorldSetup;

GameSetup makeDefaultGameSetup(uint8_t worldId);
GameSetup makeFarmGameSetup(uint8_t worldId, const FarmLayout& farm);
