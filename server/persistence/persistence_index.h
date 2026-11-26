#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "dtos.h"

struct GameIndexPlayerDTO {
    std::string characterName;
    uint16_t hairModel = 0;
    uint16_t shirtModel = 0;
    uint16_t pantsModel = 0;
    Color hairColor{0, 0, 0};
    Color pantsColor{0, 0, 0};
    std::optional<PlayerDataLocal> initialPlayer;
};

struct GameIndexDTO {
    std::string id;
    std::string farmName;
    std::string mapPath;
    std::vector<std::string> users;
    bool isMultiplayer = false;
    std::map<std::string, GameIndexPlayerDTO> playerPreviews;
};
