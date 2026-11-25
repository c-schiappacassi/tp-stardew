#include "acceptor.h"

#include "server_constants.h"

#include <iostream>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "liberror.h"

Acceptor::Acceptor(Socket& listenSocket, GameManager& manager):listenSocket(listenSocket), manager(manager) {}

void Acceptor::run() {
    while (should_keep_running()) {
        try {
            Socket clientSocket = listenSocket.accept();
            reap();
            auto handler = std::make_unique<ClientHandler>(std::move(clientSocket), manager);
            clients.push_back(std::move(handler));
            clients.back()->start();

        } catch (const LibError& e) {
            if (should_keep_running()) {
                std::cerr << server_constants::ACCEPT_ERROR_PREFIX << e.what() << server_constants::NEWLINE;
            }
            break;
        } catch (const std::exception& e) {
            std::cerr << server_constants::UNEXPECTED_ERROR_IN_ACCEPTOR_PREFIX << e.what() << server_constants::NEWLINE;
            break;
        } catch (...) {
            std::cerr << server_constants::UNKNOWN_ERROR_IN_ACCEPTOR_PREFIX;
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
            std::cerr << server_constants::ERROR_JOINING_CLIENT_DURING_REAP_PREFIX << e.what() << server_constants::NEWLINE;
        } catch (...) {
            std::cerr << server_constants::UNKNOWN_ERROR_JOINING_CLIENT_DURING_REAP_PREFIX;
        }

        return true;
    });
}

void Acceptor::clear() {
    for (const std::unique_ptr<ClientHandler>& client : clients) {
        try {
            client->stop();
        } catch (const std::exception& e) {
            std::cerr << server_constants::ERROR_STOPPING_CLIENTHANDLER_PREFIX << e.what() << server_constants::NEWLINE;
        } catch (...) {
            std::cerr << server_constants::UNKNOWN_ERROR_STOPPING_CLIENTHANDLER_PREFIX;
        }
    }

    for (const std::unique_ptr<ClientHandler>& client : clients) {
        try {
            client->join();
        } catch (const std::exception& e) {
            std::cerr << server_constants::ERROR_JOINING_CLIENTHANDLER_PREFIX << e.what() << server_constants::NEWLINE;
        } catch (...) {
            std::cerr << server_constants::UNKNOWN_ERROR_JOINING_CLIENTHANDLER_PREFIX;
        }
    }

    clients.clear();
}

void Acceptor::stop() {
    Thread::stop();
    try {
        listenSocket.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
    } catch (...) {}
}
