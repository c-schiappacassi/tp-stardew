#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <stdexcept>

#include "farm/loading/farm_toml_reader.h"
#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto> [--farm archivo.toml]\n"; //por ahroa asi para probar farm
        return 1;
    }
    try {
        std::string farmPath;
        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--farm") {
                if (!farmPath.empty() || i + 1 == argc || std::string(argv[i + 1]).starts_with("--")) {
                    throw std::invalid_argument("--farm requiere un unico archivo TOML");
                }
                farmPath = argv[++i];
            } else {
                throw std::invalid_argument("Argumento desconocido: " + argument);
            }
        }
        std::optional<FarmLayout> farm;
        if (!farmPath.empty()) farm = FarmTomlReader{}.read(farmPath);
        GameManager manager({}, {}, farm);

        Socket listener(argv[1]);
        Acceptor acceptor(listener, manager);
        acceptor.start();
        std::cout << "Servidor escuchando en " << argv[1] << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
        }

        acceptor.stop();
        acceptor.join();
        manager.stopAll();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error del servidor: " << e.what() << "\n";
        return 1;
    }
}
