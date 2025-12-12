#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <vector>

namespace fixtures {

class Queue {
public:
    void push(std::string s) {
        {
            std::lock_guard<std::mutex> lock(mu_);
            q_.push_back(std::move(s));
        }
        cv_.notify_one();
    }

    bool pop(std::string &out) {
        std::unique_lock<std::mutex> lock(mu_);
        cv_.wait(lock, [&] { return !q_.empty() || shutdown_; });
        if (q_.empty()) {
            return false;
        }
        out = std::move(q_.front());
        q_.pop_front();
        return true;
    }

    void shutdown() {
        shutdown_ = true;
        cv_.notify_all();
    }

private:
    std::mutex mu_;
    std::condition_variable cv_;
    std::deque<std::string> q_;
    bool shutdown_{false};
};

class WorkerPool {
public:
    explicit WorkerPool(int n) : threads_(n) {
        for (int i = 0; i < n; ++i) {
            threads_[i] = std::thread([this] { this->loop_(); });
        }
    }

    ~WorkerPool() {
        q_.shutdown();
        for (auto &t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    void submit(std::string s) { q_.push(std::move(s)); }

    size_t processed() const { return processed_; }

private:
    void loop_() {
        std::string item;
        while (q_.pop(item)) {
            processed_++;
            if (item.size() % 7 == 0) {
                q_.shutdown();
            }
        }
    }

    Queue q_;
    std::vector<std::thread> threads_;
    size_t processed_{0};
};

}

int main() {
    spdlog::info("fixture_complex_concurrency start");

    fixtures::WorkerPool pool(4);
    for (int i = 0; i < 100; ++i) {
        pool.submit(std::to_string(i));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    spdlog::info("fixture_complex_concurrency end {}", pool.processed());
    return 0;
}
