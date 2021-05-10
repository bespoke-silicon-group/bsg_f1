#pragma once
#include "IPNSWRunner.hpp"
#include "IPNSWResultReader.hpp"

namespace ipnsw {
    class GreedyWalkResultReader : public IPNSWResultReader {
    public:
        void readResults(const IPNSWRunner & runner) {
            HammerBlade::Ptr hb = HammerBlade::Get();
            int v_curr;
            float d_curr;

            hb->read(runner.v_curr_dev(0), &v_curr, sizeof(int));
            hb->read(runner.d_curr_dev(0), &d_curr, sizeof(float));

            std::cout << "Greedy walk (v_curr,d_curr) = "
                      << "(" << v_curr << "," << d_curr << ")"
                      << std::endl;
        }
    };
}
