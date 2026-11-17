#pragma once

#include <string>
#include <vector>

struct PartidaIndexDTO {
    std::string id;
    std::string nombreGranja;
    std::vector<std::string> usuarios;
};
