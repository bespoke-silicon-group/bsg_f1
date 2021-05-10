#pragma once
#include <tuple>
#include <vector>
namespace ipnsw {
    using GreedyWalkResult = std::pair<float, int>;
    extern std::vector<GreedyWalkResult> GREEDY_WALK_RESULTS;
    static constexpr int GWR_DIST = 0;
    static constexpr int GWR_VERT = 1;
}
