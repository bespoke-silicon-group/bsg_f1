#pragma once
#include "IPNSWFactory.hpp"
#include "IProductUBmkKernelRunner.hpp"
#include "IProductUBmkResultReader.hpp"
#include "IProductUBmkFactory.hpp"
#include "IProductUBmkParallelKernelRunner.hpp"

namespace ipnsw {
    class IProductUBmkParallelFactory : public IProductUBmkFactory {
    public:
        IProductUBmkParallelFactory(int itertions = 10):
            IProductUBmkFactory(itertions) {
        }

    private:
        IPNSWKernelRunner *_KernelRunner() const { return new IProductUBmkParallelKernelRunner(_iterations); }

    };
}

