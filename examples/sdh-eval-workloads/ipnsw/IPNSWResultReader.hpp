#pragma once
#include "HammerBlade.hpp"
namespace ipnsw {
    class IPNSWRunner;

    class IPNSWResultReader {
    protected:
        using HammerBlade = hammerblade::host::HammerBlade;

    public:
        virtual void readResults(const IPNSWRunner & runner) {}
    };
}
