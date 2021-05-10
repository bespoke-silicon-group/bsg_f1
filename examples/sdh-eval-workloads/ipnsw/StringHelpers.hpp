#pragma once
#include <string>
#include <sstream>

namespace ipnsw {
    static bool startswith(const std::string &st, const std::string &prefix) {
        return st.rfind(prefix, 0) == 0;
    }

    template <typename T>
    T from_string(const std::string &str) {
        std::stringstream ss(str);
        T v;
        ss >> v;
        return v;
    }
}
