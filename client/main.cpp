#include <QApplication>
#include <exception>
#include <iostream>

#include "game.h"
#include "qt/main_window.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto>\n";
        return 1;
    }
    try {
        QApplication app(argc, argv);
        NetworkManager net(argv[1], argv[2]);
        MainWindow menu(net);
        menu.showMaximized();
        const int result = QApplication::exec();
        if (menu.gameStatus()) {
            std::cout << "Partida: " << menu.gameStatus()->gameId << '\n';
            const ConstMapData map = net.recvMap();
            Game game(net, menu.userName(), *menu.gameStatus(), map);
            game.exe();
        }
        return result;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
