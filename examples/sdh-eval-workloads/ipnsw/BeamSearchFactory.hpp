#pragma once
#include "IPNSWFactory.hpp"
#include "BeamSearchKernelRunner.hpp"
#include "BeamSearchResultReader.hpp"
namespace ipnsw {
    class BeamSearchFactory : public IPNSWFactory {
    private:
        IPNSWKernelRunner *_KernelRunner() const { return new BeamSearchKernelRunner; }
        IPNSWResultReader *_ResultReader() const { return new BeamSearchResultReader; }
    };
}
