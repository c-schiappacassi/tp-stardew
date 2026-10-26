#include <exception>
#include <iostream>
#include <string>

#include "game_manager.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }
    try {
        GameManager manager;

        for (int i = 1; i < argc; ++i) {
            const std::string id = manager.createGame(argv[i]);
            std::cout << "Partida " << id<< " escuchando en " << argv[i] << '\n';
        }

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
        }

        manager.stopAll();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error del servidor: " << e.what() << "\n";
        return 1;
    }
}