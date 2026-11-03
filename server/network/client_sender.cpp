#include "client_sender.h"

#include <sys/socket.h>
#include <utility>

ClientSender::ClientSender(ServerProtocol& protocol, std::shared_ptr<OutQueue> output,
                           Socket& socket):
        protocol(protocol), output(std::move(output)), socket(socket) {}

void ClientSender::run() {
    try {
        while (should_keep_running()) {
            protocol.sendStatus(output->pop());
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception&) {
        // Wake the receiver so it can unregister a client that stopped reading.
        try {
            socket.shutdown(SHUT_RDWR);
        } catch (const std::exception&) {
        }
    }
}
