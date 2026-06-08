#pragma once

#include <spdlog/common.h>

#include <map>
#include <mutex>


namespace spdlog {

class SPDLOG_API log_attributes {
public:
    using attr_map_t = std::map<std::string, std::string>;
    using key_t = attr_map_t::key_type;
    using value_t = attr_map_t::mapped_type;
    using const_iter = attr_map_t::const_iterator;

    class SPDLOG_API log_attr_context {
    public:
        log_attr_context(log_attributes& parent, attr_map_t const& attrs)
            : parent_(parent),
              tmp_attrs_{attrs} {
            parent_.put(attrs);
        }
        explicit log_attr_context(log_attributes& parent)
            : log_attr_context(parent, {}) {}

        ~log_attr_context() {
            for (auto&& key_value : tmp_attrs_) parent_.remove(key_value.first);
        }

    private:
        log_attributes& parent_;
        attr_map_t tmp_attrs_;
    };

    log_attributes() = default;
    log_attributes(const log_attributes& other) { put(other.get_map()); }
    log_attributes& operator=(const log_attributes& other) {
        if (this != &other) {
            clear();
            put(other.get_map());
        }
        return *this;
    }

    void put(attr_map_t const& attributes) {
        auto lck = lock();
        for (auto const& attribute : attributes) attrs.insert_or_assign(attribute.first, attribute.second);
    }
    void put(const key_t& key, const value_t& value) { put({{key, value}}); }

    void remove(const key_t& key) {
        auto lck = lock();
        auto value_it = attrs.find(key);
        if (value_it != attrs.end()) {
            attrs.erase(key);
        }
    }

    void clear() {
        auto lck = lock();
        attrs.clear();
    }

    bool empty() const {
        auto lck = lock();
        return attrs.empty();
    }

    // return {true, iter} if found, {false, end} otherwise
    std::pair<bool, const_iter> const get(key_t const& key) const {
        auto lck = lock();
        auto value_it = attrs.find(key);

        return std::make_pair(value_it != attrs.end(), value_it);
    }

    // provide a local copy of the attributes to be free of race issues
    // alternative is to lock the attr_map while the formatter iterates over it
    attr_map_t get_map() const {
        auto lck = lock();
        return attrs;
    }

    // RAII-wrapper that inserts a couple of attributes and removes them upon destruction
    log_attr_context scoped_ctx(attr_map_t const& attributes) { return log_attr_context(*this, attributes); }
    log_attr_context scoped_ctx(key_t key, value_t value) { return log_attr_context(*this, {{key, value}}); }

    bool empty() {
        auto lck = lock();
        return attrs.empty();
    }

private:
    std::lock_guard<std::mutex> lock() const { return std::lock_guard(attrs_mtx); }

    mutable std::mutex attrs_mtx;
    attr_map_t attrs;
};

}  // namespace spdlog