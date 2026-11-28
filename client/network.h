#pragma once

#include <atomic>
#include <optional>

#include "client_constants.h"
#include "client_protocol.h"
#include "dtos.h"
#include "queue.h"
#include "recv_thread.h"
#include "send_thread.h"

class NetworkManager {
private:
    Queue<std::shared_ptr<CommandDTO>> send_queue{client_constants::SEND_QUEUE_SIZE};
    Queue<ServerStatus> recv_queue{client_constants::RECV_QUEUE_SIZE};
    ClientProtocol protocolo;
    std::atomic<bool> connected{true};
    SendThread sender;
    RecvThread receiver;
    bool started = false;
    bool closed = false;

public:
    NetworkManager(const std::string& host, const std::string& port):
            protocolo(Socket(host.c_str(), port.c_str())),
            sender(send_queue, protocolo, connected),
            receiver(recv_queue, protocolo, connected) {}

    ~NetworkManager() { quit(); }

    void start() {
        if (started || closed) throw std::logic_error("La conexion no se puede reiniciar");
        sender.start();
        try {
            receiver.start();
        } catch (...) {
            sender.stop();
            sender.join();
            throw;
        }
        started = true;
    }

    bool isConnected() const { return connected; }

    void push_to_send(const std::shared_ptr<CommandDTO>& dto) {
        if (!connected) return;
        send_queue.try_push(dto);
    }

    std::optional<ServerStatus> try_pop_recv() {
        ServerStatus status;
        try {
            if (recv_queue.try_pop(status)) return status;
        } catch (const ClosedQueue&) {
            connected = false;
        }
        return std::nullopt;
    }

    void quit() {
        if (closed) return;
        closed = true;
        connected = false;
        if (started) {
            sender.stop();
            receiver.stop();
        }
        try {
            protocolo.shutdown();
        } catch (const std::exception&) {
        }
        if (started) {
            sender.join();
            receiver.join();
        }
    }

    ServerStatus handshake(const CommandDTO& dto) {
        protocolo.send(dto);
        ServerStatus status;
        protocolo.recv(status);
        return status;
    }

    qtStatus handshakeQt(const CommandDTO& dto) {
        protocolo.send(dto);
        qtStatus status;
        protocolo.recv(status);
        return status;
    }

    const ConstMapData recvMap(){
        ConstMapData map;
        protocolo.recv(map);
        return map;
    };
};
