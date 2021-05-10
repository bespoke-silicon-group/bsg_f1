#pragma once
#include "IPNSWRunner.hpp"
#include "IPNSWResultReader.hpp"
#include "GreedyWalkResults.hpp"

namespace ipnsw {
    class BeamSearchResultReader : public IPNSWResultReader {
    public:
        void readResults(const IPNSWRunner & runner) {
            HammerBlade::Ptr hb = HammerBlade::Get();

            hb_mc_eva_t grp = 0;
            int n_results;
            hb->read(runner.n_results_dev(grp), &n_results, sizeof(int));

            std::vector<GreedyWalkResult> results(n_results);
            hb->push_read(runner.results_dev(grp), &results[0], n_results * sizeof(GreedyWalkResult));
            hb->sync_read();

            std::cout << "Beam search:" << std::endl;
            for (auto & r : results) {
                std::cout << "{" << std::get<0>(r) << "," << std::get<1>(r) << "}" << std::endl;
            }
        }
    };
}
