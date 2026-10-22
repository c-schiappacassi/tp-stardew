#include <exception>
#include <iostream>
#include <string>

#include "Acceptor.h"
#include "game_manager.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }
    try {
        Socket listenSocket(argv[1]);
        GameManager manager;
        Acceptor acceptor(listenSocket, manager);

        acceptor.start();

        std::cout << "Servidor escuchando en el puerto " << argv[1]<< ". Escribi q para salir.\n";

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
            
        }

        acceptor.stop();
        acceptor.join();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error del servidor: " << e.what() << "\n";
        return 1;
    }
}