#include "bsg_manycore_regression.h"
#include "ipnsw.hpp"
#include "HammerBlade.hpp"
#include "Graph500Data.hpp"
#include "Graph.hpp"
#include "IO.hpp"
#include "IPNSWGraph.hpp"
#include "IPNSWRunner.hpp"
#include "IProductUBmkKernelRunner.hpp"
#include "IProductUBmkResultReader.hpp"
#include "IProductUBmkFactory.hpp"
#include "IProductUBmkParallelFactory.hpp"
#include "BeamSearchKernelRunner.hpp"
#include "BeamSearchResultReader.hpp"
#include "BeamSearchFactory.hpp"
#include "GreedyWalkKernelRunner.hpp"
#include "GreedyWalkResultReader.hpp"
#include "GreedyWalkFactory.hpp"
#include "GreedyWalkResults.hpp"
#include "StringHelpers.hpp"
#include <iostream>
#include <memory>

using namespace ipnsw;

int Main(int argc, char *argv[])
{
    Parser args;
    args.parse(argc, argv);

    std::cout << args.str() << std::endl;

    std::unique_ptr<IPNSWRunner> runner;
    std::unique_ptr<IPNSWFactory> factory;

    IPNSWRunnerConfig cfg;
    cfg.grid_x() = args.grid_x();
    cfg.grid_y() = args.grid_y();
    cfg.grp_x()  = args.grp_x();
    cfg.grp_y()  = args.grp_y();

    if (ipnsw::startswith(args.version(), "greedy_walk")) {
        factory = std::unique_ptr<IPNSWFactory>(new GreedyWalkFactory);
    } else if (ipnsw::startswith(args.version(), "beam_search")) {
        factory = std::unique_ptr<IPNSWFactory>(new BeamSearchFactory);
    } else if (ipnsw::startswith(args.version(), "iproduct_ubmk")) {
        /* parse the number of inner products */
        std::cout << "num inner products " << args.num_iproducts() << std::endl;
        int n_iproducts = args.num_iproducts();

        bool parallel = args.version().find("parallel") != std::string::npos;
        if (parallel) {
            factory = std::unique_ptr<IPNSWFactory>(new IProductUBmkParallelFactory(n_iproducts));
        } else {
            factory = std::unique_ptr<IPNSWFactory>(new IProductUBmkFactory(n_iproducts)) ;
        }

    } else if (args._version == "debug") {
        /* just for debugging */
        std::cout << "--num-iproducts=" << args.num_iproducts() << std::endl;
        std::cout << "--queries=" << std::endl;
        std::cout << "--group-x=" << args.grp_x() << std::endl;
        std::cout << "--group-y=" << args.grp_y() << std::endl;
        auto do_queries = args.do_queries();
        for (auto q : do_queries) {
            std::cout << q << " ";
        }
        std::cout << std::endl;
        return 0;
    } else {
        return 0;
    }

    runner = std::unique_ptr<IPNSWRunner>(new IPNSWRunner(args, factory, cfg));
    runner->run();

    return 0;
}

declare_program_main("IPNSW", Main);
