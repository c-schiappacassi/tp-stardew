#include <exception>
#include <iostream>
#include <string>

#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }
    try {
        GameManager manager;

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
