// OpenAI Codex, GPL-2.0-or-later. Portable auto-reset worker wakeup.
#pragma once
#include <condition_variable>
#include <mutex>
#include <chrono>
namespace friends {
class WakeEvent {
    std::mutex mutex;
    std::condition_variable condition;
    bool pending=false;
public:
    void signal(){std::lock_guard<std::mutex> lock(mutex);pending=true;condition.notify_one();}
    void wait(bool active){
        std::unique_lock<std::mutex> lock(mutex);
        if(active)condition.wait_for(lock,std::chrono::milliseconds(5),[&]{return pending;});
        else condition.wait(lock,[&]{return pending;});
        pending=false;
    }
};
}
