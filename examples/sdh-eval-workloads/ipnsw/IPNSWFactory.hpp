#pragma once
#include "IPNSWKernelRunner.hpp"
#include "IPNSWResultReader.hpp"
namespace ipnsw {
    class IPNSWFactory {
    public:
        std::unique_ptr<IPNSWKernelRunner> KernelRunner()const {
            return std::unique_ptr<IPNSWKernelRunner>(_KernelRunner());
        }
        std::unique_ptr<IPNSWResultReader> ResultReader()const {
            return std::unique_ptr<IPNSWResultReader>(_ResultReader());
        }
    protected:
        virtual IPNSWKernelRunner* _KernelRunner()const = 0;
        virtual IPNSWResultReader* _ResultReader()const = 0;
    };
}
