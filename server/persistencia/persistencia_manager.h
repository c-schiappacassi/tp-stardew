#pragma once

#include <string>
#include <vector>

#include "partidas_index.h"

class PersistenciaManager {
private:
    std::string rutaIndice;
    std::vector<PartidaIndexDTO> partidas;

    void cargarIndice();
    void guardarIndice() const;

public:
    explicit PersistenciaManager(
        const std::string& ruta = "partidas.toml");

    std::vector<PartidaIndexDTO> obtenerPartidas() const;

    void registrarPartida(const std::string& id,
                         const std::string& nombreGranja,
                         const std::string& usuario);

    void registrarUsuario(const std::string& id,
                          const std::string& usuario);
};
