#include "queue.h"
#include "send_thread.h"
#include "recv_thread.h"
#include "dtos.h"
#include <optional>
#include "client_protocol.h"

#define MAX_SIZE_QUEUE 100

class NetworkManager {
private:
    Queue<GameDTO> send_queue;
    Queue<ServerStatus> recv_queue;
    ClientProtocol protocolo;
    SendThread sender;
    RecvThread receiver;

public:
    NetworkManager(const std::string& host, const std::string& port)
        : send_queue(MAX_SIZE_QUEUE),
          recv_queue(MAX_SIZE_QUEUE), 
          protocolo(host, port), 
          sender(send_queue, protocolo), 
          receiver(recv_queue, protocolo) {}

    void start() {
        sender.run();
        receiver.run();
    }

    void push_to_send(const GameDTO& dto) {
        send_queue.push(dto);
    }

    std::optional<ServerStatus> try_pop_recv() {
        ServerStatus status;
        if (recv_queue.try_pop(status)) {
            return status;
        }
        return std::nullopt;
    }

    void quit() {
        sender.stop();
        receiver.stop();

        sender.join();
        receiver.join();
    }
};