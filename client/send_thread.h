#pragma once

#include "thread.h"
#include "queue.h"
#include "dtos.h"
#include "client_protocol.h"

class SendThread : public Thread {
private:
    Queue<std::shared_ptr<CommandDTO>>& send_queue;
    ClientProtocol& protocolo;
    bool shoud_keep_running;

public:
    SendThread(Queue<std::shared_ptr<CommandDTO>>& queue, ClientProtocol& prot)
        : send_queue(queue), protocolo(prot) {}

    void run() override {
        this->shoud_keep_running = true;
        try {
            while (shoud_keep_running) {
                std::shared_ptr<CommandDTO> dto = send_queue.pop();                
                protocolo.send(*dto);
            }
        } catch (const ClosedQueue&) {
            // TODO
        } catch (const std::exception& e) {
            // TODO
        }
    }

    void stop() override {
        this->shoud_keep_running = false;
        Thread::stop();
        send_queue.close();
    }
};
