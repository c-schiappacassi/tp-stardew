#pragma once

#include <atomic>
#include "thread.h"
#include "queue.h"
#include "dtos.h"
#include "client_protocol.h"

class RecvThread : public Thread {
private:
    Queue<ServerStatus>& recv_queue;
    ClientProtocol& protocolo;
    std::atomic<bool>& connected;

public:
    RecvThread(Queue<ServerStatus>& queue, ClientProtocol& prot, std::atomic<bool>& connected):
            recv_queue(queue), protocolo(prot), connected(connected) {}

    void run() override {
        try {
            while (should_keep_running()) {
                ServerStatus status;
                protocolo.recv(status);
                recv_queue.push(status);
            }
        } catch (const std::exception&) {
        }
        connected = false;
    }

    void stop() override {
        Thread::stop();
        recv_queue.close();
    }
};
