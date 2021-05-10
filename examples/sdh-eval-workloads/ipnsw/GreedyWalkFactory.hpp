#pragma once
#include "IPNSWFactory.hpp"
#include "GreedyWalkKernelRunner.hpp"
#include "GreedyWalkResultReader.hpp"

namespace ipnsw {
    class GreedyWalkFactory : public IPNSWFactory {
    private:
        IPNSWKernelRunner *_KernelRunner() const { return new GreedyWalkKernelRunner; }
        IPNSWResultReader *_ResultReader() const { return new GreedyWalkResultReader; }
    };
}
