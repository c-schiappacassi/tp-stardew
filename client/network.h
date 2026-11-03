#pragma once

#include "queue.h"
#include "send_thread.h"
#include "recv_thread.h"
#include "dtos.h"
#include <optional>
#include "client_protocol.h"

#define MAX_SIZE_QUEUE 100

class NetworkManager {
private:
    Queue<std::shared_ptr<CommandDTO>> send_queue;
    Queue<ServerStatus> recv_queue;
    // TODO:TBD si protocolo deberia ser dueño del socket o deberia tenerlo network
    Socket socket;
    ClientProtocol protocolo;
    SendThread sender;
    RecvThread receiver;

public:
    NetworkManager(const std::string& host, const std::string& port)
        : send_queue(MAX_SIZE_QUEUE),
          recv_queue(MAX_SIZE_QUEUE), 
          socket(host.c_str(), port.c_str()),
          protocolo(socket),
          sender(send_queue, protocolo), 
          receiver(recv_queue, protocolo, socket) {}

    void start() {
        sender.start();
        receiver.start();
    }

   
    void push_to_send(std::shared_ptr<CommandDTO> dto) {
        try {
            send_queue.try_push(dto);
        } catch (const ClosedQueue&) {
            // TODO
        }
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
    
    // TEMP
    ServerStatus handshake(const CommandDTO& dto) {
        protocolo.send(dto);
        ServerStatus status;
        protocolo.recv(status);
        return status;
    }
    //para ListGames y Checkcode que no respondern con ServerStatus sino qtstatus
    qtStatus handshakeQt(const CommandDTO& dto) {
        protocolo.send(dto);
        qtStatus status;
        protocolo.recv(status);
        return status;
    }
};
