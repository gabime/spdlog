#include <cstdlib>
#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace fixtures {

struct Connection {
    explicit Connection(std::string name) : name_(std::move(name)) {
        buffer_ = static_cast<char *>(std::malloc(32));
        if (buffer_) {
            buffer_[0] = '\0';
        }
    }

    ~Connection() {
        if (buffer_) {
            std::free(buffer_);
        }
    }

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    Connection(Connection &&other) noexcept : name_(std::move(other.name_)), buffer_(other.buffer_) {
        other.buffer_ = nullptr;
    }

    Connection &operator=(Connection &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        name_ = std::move(other.name_);
        buffer_ = other.buffer_;
        other.buffer_ = nullptr;
        return *this;
    }

    void send(const std::string &msg) {
        if (buffer_) {
            std::snprintf(buffer_, 32, "%s", msg.c_str());
        }
    }

    const char *buffer() const { return buffer_; }

private:
    std::string name_;
    char *buffer_{nullptr};
};

class ConnectionPool {
public:
    Connection &get_or_create(const std::string &key) {
        auto it = conns_.find(key);
        if (it == conns_.end()) {
            it = conns_.emplace(key, std::unique_ptr<Connection>(new Connection(key))).first;
        }
        return *it->second;
    }

    Connection *borrow_raw(const std::string &key) {
        auto &c = get_or_create(key);
        borrowed_.push_back(&c);
        return &c;
    }

    void drop(const std::string &key) { conns_.erase(key); }

    std::string peek_borrowed(size_t i) const {
        if (i >= borrowed_.size()) {
            return {};
        }
        const Connection *c = borrowed_[i];
        return c ? std::string(c->buffer() ? c->buffer() : "") : std::string();
    }

private:
    std::map<std::string, std::unique_ptr<Connection>> conns_;
    std::vector<Connection *> borrowed_;
};

}

int main() {
    spdlog::info("fixture_complex_ownership start");

    fixtures::ConnectionPool pool;

    auto *c = pool.borrow_raw("alpha");
    c->send("hello");

    pool.drop("alpha");

    c->send("world");

    spdlog::info("fixture_complex_ownership end {}", pool.peek_borrowed(0));
    return 0;
}
