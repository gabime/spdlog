#pragma once

#include <string>
#include <set>
#include <map>

struct binaryPredicate {
    bool operator() (const std::string& l, const std::string& r) const {
        if (l.size() != r.size()) {
            return l.size() < r.size();
        }

        return l < r;
    }
};

using name_set_t = std::set<std::string, binaryPredicate>;
using name_map_t = std::map<std::string, std::size_t, binaryPredicate>;