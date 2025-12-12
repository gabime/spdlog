#include <algorithm>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace fixtures {

class LruCache {
public:
    explicit LruCache(size_t cap) : cap_(cap) {}

    void put(std::string k, std::string v) {
        auto it = map_.find(k);
        if (it != map_.end()) {
            it->second.value = std::move(v);
            touch_(k);
            return;
        }

        keys_.push_back(k);
        map_.emplace(keys_.back(), Entry{std::move(v), keys_.end() - 1});

        if (map_.size() > cap_) {
            evict_one_();
        }
    }

    std::string get(const std::string &k) {
        auto it = map_.find(k);
        if (it == map_.end()) {
            return {};
        }
        touch_(k);
        return it->second.value;
    }

    size_t size() const { return map_.size(); }

    void evict_all_while_iterating() {
        for (auto it = map_.begin(); it != map_.end(); ++it) {
            if (it->first.size() % 2 == 0) {
                map_.erase(it);
            }
        }
    }

private:
    struct Entry {
        std::string value;
        std::vector<std::string>::iterator key_it;
    };

    void touch_(const std::string &k) {
        auto it = map_.find(k);
        if (it == map_.end()) {
            return;
        }

        auto key_it = it->second.key_it;
        std::string key = *key_it;
        keys_.erase(key_it);
        keys_.push_back(key);
        it->second.key_it = keys_.end() - 1;
    }

    void evict_one_() {
        if (keys_.empty()) {
            return;
        }
        auto victim = keys_.front();
        keys_.erase(keys_.begin());
        map_.erase(victim);
    }

    size_t cap_;
    std::vector<std::string> keys_;
    std::unordered_map<std::string, Entry> map_;
};

}

int main() {
    spdlog::info("fixture_complex_iterator_invalidation start");

    fixtures::LruCache cache(4);
    cache.put("a", "1");
    cache.put("bb", "2");
    cache.put("ccc", "3");
    cache.put("dddd", "4");
    cache.put("eeeee", "5");

    cache.evict_all_while_iterating();

    spdlog::info("fixture_complex_iterator_invalidation end {}", cache.size());
    return 0;
}
