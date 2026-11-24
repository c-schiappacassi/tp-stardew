#include "client_sender.h"

#include <sys/socket.h>
#include <utility>

ClientSender::ClientSender(ServerProtocol& protocol, std::shared_ptr<OutQueue> output):
        protocol(protocol), output(std::move(output)) {}

void ClientSender::run() {
    try {
        while (should_keep_running()) {
            protocol.sendStatus(output->pop());
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception&) {}

    try {
        protocol.shutdown();
    } catch (const std::exception&) {}
}
