#include "acceptor.h"

#include <iostream>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "../common/liberror.h"

Acceptor::Acceptor(Socket& listenSocket, GameManager& manager)
    : listenSocket(listenSocket), manager(manager) {}

void Acceptor::run() {
    while (should_keep_running()) {
        try {
            Socket clientSocket = listenSocket.accept();
            reap();
            std::unique_ptr<ClientHandler> handler =
                std::make_unique<ClientHandler>(std::move(clientSocket),
                                                manager);
            clients.push_back(std::move(handler));
            clients.back()->start();

        } catch (const LibError& e) {
            if (should_keep_running()) {
                std::cerr << "Accept error: " << e.what() << "\n";
            }
            break;
        } catch (const std::exception& e) {
            std::cerr << "Unexpected error in Acceptor: " << e.what() << "\n";
            break;
        } catch (...) {
            std::cerr << "Unknown error in Acceptor.\n";
            break;
        }
    }

    clear();
}

void Acceptor::reap() {
    clients.remove_if([](const std::unique_ptr<ClientHandler>& client) {
        if (client->is_alive()) {
            return false;
        }

        try {
            client->join();
        } catch (const std::exception& e) {
            std::cerr << "Error joining client during reap: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Unknown error joining client during reap.\n";
        }

        return true;
    });
}

void Acceptor::clear() {
    for (const std::unique_ptr<ClientHandler>& client : clients) {
        try {
            client->stop();
        } catch (const std::exception& e) {
            std::cerr << "Error stopping ClientHandler: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Unknown error stopping ClientHandler.\n";
        }
    }

    for (const std::unique_ptr<ClientHandler>& client : clients) {
        try {
            client->join();
        } catch (const std::exception& e) {
            std::cerr << "Error joining ClientHandler: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Unknown error joining ClientHandler.\n";
        }
    }

    clients.clear();
}

void Acceptor::stop() {
    Thread::stop();
    try {
        listenSocket.shutdown(SHUT_RDWR);
    } catch (const std::exception& e) {
    } catch (...) {}
}
