#pragma once

#include <atomic>
#include <memory>
#include <sys/socket.h>
#include "thread.h"
#include "queue.h"
#include "dtos.h"
#include "client_protocol.h"

class SendThread : public Thread {
private:
    Queue<std::shared_ptr<CommandDTO>>& send_queue;
    ClientProtocol& protocolo;
    Socket& socket;
    std::atomic<bool>& connected;

public:
    SendThread(Queue<std::shared_ptr<CommandDTO>>& queue, ClientProtocol& prot,
               Socket& socket, std::atomic<bool>& connected):
            send_queue(queue), protocolo(prot), socket(socket), connected(connected) {}

    void run() override {
        try {
            while (should_keep_running()) {
                const auto dto = send_queue.pop();
                protocolo.send(*dto);
            }
        } catch (const ClosedQueue&) {
        } catch (const std::exception&) {
            connected = false;
            try {
                socket.shutdown(SHUT_RDWR);
            } catch (const std::exception&) {
            }
        }
    }

    void stop() override {
        Thread::stop();
        send_queue.close();
    }
};
