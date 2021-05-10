#pragma once
#include "IPNSWKernelRunner.hpp"
#include "IProductUBmkKernelRunner.hpp"
#include "IPNSWRunner.hpp"
#include "HammerBlade.hpp"
#include <algorithm>

namespace ipnsw {
    class IProductUBmkParallelKernelRunner : public IProductUBmkKernelRunner {
    public:
        IProductUBmkParallelKernelRunner(int iterations = 10) :
            IProductUBmkKernelRunner(iterations) {
        }

    private:
        using HammerBlade = hammerblade::host::HammerBlade;

        void beforeLaunchKernel(const IPNSWRunner &runner) {
            HammerBlade::Ptr _hb = HammerBlade::Get();
            
            _visit.clear();
            
            for (int i = 0; i < _iterations * runner.numGroups(); ++i) {
                _visit.push_back((i*3) % runner.db().size());
            }
            std::random_shuffle(_visit.begin(), _visit.end());
            
            _visit_dev = _hb->alloc(sizeof(int) * _visit.size());

            std::cout << "beforeLaunchKernel called: _visit_dev = " << std::hex << _visit_dev << std::endl;
            std::cout << std::dec;

            _hb->push_write(_visit_dev, &_visit[0], sizeof(int) * _visit.size());
        }

        std::vector<hb_mc_eva_t> argv(const IPNSWRunner & runner) const {
            std::cout << "Called" << std::endl;
            std::vector<hb_mc_eva_t> argv = {
                runner.db_dev(), // database
                runner.query_dev(0), // query                
                static_cast<hb_mc_eva_t>(_iterations), // number of inner products
                _visit_dev, // vectors to visit
            };
            return argv;
        }

        void afterLaunchKernel(const IPNSWRunner &runner) {
            HammerBlade::Ptr _hb = HammerBlade::Get();
            _hb->free(_visit_dev);
            _visit.clear();
        }

        virtual Dim gd(const IPNSWRunner &runner) const {
            return Dim(runner.cfg().grid_x(),runner.cfg().grid_y());
        }

        virtual Dim tgd(const IPNSWRunner &runner) const {
            return Dim(runner.cfg().grp_x(),runner.cfg().grp_y());
        }

        hb_mc_eva_t          _visit_dev;
        std::vector<int>     _visit;
    };
}
