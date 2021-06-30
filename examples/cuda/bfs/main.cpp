// Copyright (c) 2019, University of Washington All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <cmath>
#include <bsg_manycore_regression.h>
#include "HammerBlade.hpp"
#include "bfs/CL.hpp"
#include "bfs/BFSGraph.hpp"
#include "bfs/BFSSparseSet.hpp"
#include "bfs/BFSDenseSet.hpp"
#include "WGraph.hpp"
#include "SparsePushBFS.hpp"

using namespace hammerblade::host;
using namespace BFS;
using namespace graph_tools;

HammerBlade::Ptr HB;

int Main(int argc, char *argv[])
{
    CL cl;
    cl.parse(argc, argv);

    WGraph g;
    int iter = cl.bfs_iteration();

    if (cl.graph_type() == "uniform") {
        g = WGraph::Uniform(cl.graph_vertices(), cl.graph_edges());
    } else if (cl.graph_type() == "graph500") {
        g = WGraph::Generate(ceil(log2(cl.graph_vertices())),cl.graph_edges());
    }

    std::vector<SparsePushBFS> stats = SparsePushBFS::RunBFS(g, cl.bfs_root(), cl.bfs_iteration());

    // load application
    HB = HammerBlade::Get();
    HB->load_application(cl.binary_path());

    // format graph on device
    BFSGraph bfsg(g);
    BFSSparseSet frontier_in(stats[iter].frontier_in(), bfsg.graph().num_nodes());
    BFSDenseSet  frontier_out(std::set<int>(),    bfsg.graph().num_nodes());
    BFSDenseSet  visited_io(stats[iter].visited_in(),   bfsg.graph().num_nodes());

    bfsg.formatOnDevice();
    frontier_in.formatOnDevice();
    frontier_out.formatOnDevice();
    visited_io.formatOnDevice();

    // sync writes
    HB->sync_write();
    bsg_pr_info("Launching bfs with %d groups of shape (x=%d,y=%d)\n", cl.groups(), cl.tgx(), cl.tgy());
    HB->push_job(Dim(cl.groups(),1), Dim(cl.tgx(),cl.tgy()),
                 "bfs", bfsg.kgraph_dev(), frontier_in.dev(), frontier_out.dev(), visited_io.dev());
    HB->exec();

    // read output
    frontier_out.updateFromDevice();
    visited_io.updateFromDevice();
    HB->sync_read();

    std::set<int> frontier_out_kernel = frontier_out.setAfterUpdate();
    std::set<int> frontier_out_host = stats[iter].frontier_out();

    // check sets are equal
    // check that host contains kernel
    bool equals = true;
    for (int m : frontier_out_host) {
        auto it = frontier_out_kernel.find(m);
        if (it == frontier_out_kernel.end()) {
            bsg_pr_err("Found %d in host result but not kernel\n", m);
            equals = false;
        }
    }
    // check that kernel contains host
    for (int m : frontier_out_kernel) {
        auto it = frontier_out_host.find(m);
        if (it == frontier_out_host.end()) {
            bsg_pr_err("Found %d in kernel result but not host\n", m);
            equals = false;
        }
    }

    stats[iter].dump("bfs_stats.txt");

    return equals ? HB_MC_SUCCESS : HB_MC_FAIL;
}

declare_program_main("BFS", Main);
