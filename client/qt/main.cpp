#include <QApplication>
#include <iostream>

#include "mainwindow.h"
#include "network.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Expected: " << argv[0] << " <host> <port>\n";
        return 1;
    }
    try {
        NetworkManager net(argv[1], argv[2]);
        QApplication a(argc, argv);
        MainWindow w(net);
        w.showMaximized();
        return QApplication::exec();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}