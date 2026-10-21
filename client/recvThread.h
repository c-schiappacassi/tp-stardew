#include "thread.h"
#include "queue.h"
#include "gameDTO.h"
// #include "ClientProtocol.h"


class RecvThread : public Thread {
private:
    Queue<serverStatus>& recv_queue;
    ClientProtocol& protocolo;
    bool should_keep_running;

public:
    RecvThread(Queue<serverStatus>& queue, ClientProtocol& prot):recv_queue(queue), protocolo(prot) {}

    void run() override {
        this->should_keep_running = true;
        try {
            while (this->should_keep_running) {
                serverStatus status = protocolo.try_recv(); 
                
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