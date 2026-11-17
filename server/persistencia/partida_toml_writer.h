#pragma once

#include <filesystem>

#include "persistencia/dto.h"

class PartidaTomlWriter {
public:
    void write(const PersistenciaDTO& partida, const std::filesystem::path& archivo) const;
};
