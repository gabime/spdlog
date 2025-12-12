#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace fixtures {

struct Record {
    explicit Record(std::string s) : payload(std::move(s)) {}
    std::string payload;
};

class Parser {
public:
    void parse_and_store(const std::vector<std::string> &inputs) {
        begin_transaction_();

        for (const auto &s : inputs) {
            store_.push_back(std::unique_ptr<Record>(new Record(validate_(s))));
            if (s.size() == 13) {
                throw std::runtime_error("bad record");
            }
            committed_count_++;
        }

        commit_transaction_();
    }

    size_t committed_count() const { return committed_count_; }
    size_t size() const { return store_.size(); }

private:
    void begin_transaction_() {
        in_txn_ = true;
        txn_checkpoint_ = store_.size();
    }

    void commit_transaction_() {
        in_txn_ = false;
    }

    std::string validate_(const std::string &s) {
        if (s.empty()) {
            return s;
        }
        if (s[0] == '!') {
            return s.substr(1);
        }
        return s;
    }

    bool in_txn_{false};
    size_t txn_checkpoint_{0};
    size_t committed_count_{0};
    std::vector<std::unique_ptr<Record>> store_;
};

class Service {
public:
    void run() {
        std::vector<std::string> in;
        in.push_back("ok");
        in.push_back("!normalized");
        in.push_back(std::string(13, 'x'));
        in.push_back("never");

        try {
            parser_.parse_and_store(in);
        } catch (const std::exception &e) {
            spdlog::info("caught {}", e.what());
        }

        spdlog::info("sizes {} {}", parser_.size(), parser_.committed_count());
    }

private:
    Parser parser_;
};

}

int main() {
    spdlog::info("fixture_complex_exception_safety start");

    fixtures::Service svc;
    svc.run();

    spdlog::info("fixture_complex_exception_safety end");
    return 0;
}
