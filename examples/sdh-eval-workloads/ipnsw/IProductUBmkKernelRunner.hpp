#pragma once
#include "IPNSWKernelRunner.hpp"
#include "IPNSWRunner.hpp"

namespace ipnsw {
    class IProductUBmkKernelRunner : public IPNSWKernelRunner {
    public:
        IProductUBmkKernelRunner(int iterations = 10) :
            IPNSWKernelRunner(),
            _iterations(iterations) {
        }

    private:
        std::string kernelName(const IPNSWRunner & runner) const {
            return "inner_product_ubmk";
        }

        virtual std::vector<hb_mc_eva_t> argv(const IPNSWRunner & runner) const {
            std::vector<hb_mc_eva_t> argv = {
                runner.db_dev(), // database
                runner.query_dev(0), // query
                static_cast<hb_mc_eva_t>(_iterations), // number of inner products
            };
            return argv;
        }

    protected:
        int _iterations;
    };
}
