#pragma once

#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "partidas_index.h"
#include "persistencia/dto.h"

class PersistenciaManager {
private:
    std::filesystem::path directorio;
    std::filesystem::path archivoIndice;
    mutable std::mutex mtx;
    std::map<std::string, PartidaIndexDTO> partidas;

    void cargarIndice();
    void guardarIndice() const;

public:
    explicit PersistenciaManager(
        std::filesystem::path directorio = "persistencia");

    void registrarPartida(const std::string& id,
                          const std::string& nombreGranja,
                          const std::string& usuario);
    void registrarUsuario(const std::string& id, const std::string& usuario);
    void persistirPartida(const PersistenciaDTO& partida);
    std::vector<PartidaIndexDTO> obtenerPartidas() const;
};
