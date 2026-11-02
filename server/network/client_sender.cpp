#include "client_sender.h"

#include <iostream>
#include <memory>
#include <variant>

#include "liberror.h"
#include "protocol_errors.h"
#include "queue.h"

ClientSender::ClientSender(ServerProtocol& protocol, const std::shared_ptr<OutQueue>& outQueue): protocol(protocol), outQueue(outQueue) {}

void ClientSender::run() {
    try {
        while (should_keep_running()) {
            const OutgoingMessage message = outQueue->pop();
            sendMessage(message);
        }
    } catch (const ClosedQueue&) {
    } catch (const ConnectionClosed&) {
    } catch (const LibError&) {
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in sender: " << e.what() << "\n";
    }
}

// queda asi ya que usamos server status
void ClientSender::sendMessage(const OutgoingMessage& message) {
    protocol.sendStatus(message);
}
