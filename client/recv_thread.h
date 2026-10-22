#include "thread.h"
#include "queue.h"
#include "dtos.h"
#include "client_protocol.h"


class RecvThread : public Thread {
private:
    Queue<ServerStatus>& recv_queue;
    ClientProtocol& protocolo;
    bool should_keep_running;

public:
    RecvThread(Queue<ServerStatus>& queue, ClientProtocol& prot):recv_queue(queue), protocolo(prot) {}

    void run() override {
        this->should_keep_running = true;
        try {
            while (this->should_keep_running) {
                ServerStatus status = protocolo.try_recv(); 
                
                recv_queue.push(status);
            }
        } catch (const ClosedQueue&) {
            // TODO
        } catch (const std::exception& e) {
            // TODO
        }
    }

    void stop() override {
        this->should_keep_running = false;
        protocolo.close();
        Thread::stop();
        recv_queue.close();
    }
};