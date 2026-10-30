#include "acceptor.h"

#include <iostream>
#include <utility>

#include <sys/socket.h>


Acceptor::Acceptor(const char* servername, GameManager& manager):
        listenSocket(servername), manager(manager) {}

void Acceptor::run() {
    try {
        while (should_keep_running()) {
            Socket peer = listenSocket.accept();
            reap();
            std::unique_ptr<ClientHandler> client = std::make_unique<ClientHandler>(std::move(peer), manager);
            clients.push_back(std::move(client));
            clients.back()->start();
        }
    } catch (const std::exception& e) {
        if (should_keep_running()) {
            std::cerr << "Acceptor error: " << e.what() << "\n";
        }
    }
    clear();
}

void Acceptor::reap() {
    clients.remove_if([](const std::unique_ptr<ClientHandler>& c) {
        if (c->is_alive()) {
            return false;
        }
        c->join();
        return true;
    });
}

void Acceptor::clear() {
    for (auto& c: clients) {
        c->stop();
    }
    for (auto& c: clients) {
        c->join();
    }
    clients.clear();
}

void Acceptor::stop() {
    Thread::stop();
    try {
        listenSocket.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {}
}
