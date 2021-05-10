#pragma once
#include "IPNSWRunner.hpp"
#include "IPNSWResultReader.hpp"

namespace ipnsw {
    class IProductUBmkResultReader : public IPNSWResultReader {
    public:
        void readResults(const IPNSWRunner & runner) {
            std::cout << "Done" << std::endl;
        }
    };
}
