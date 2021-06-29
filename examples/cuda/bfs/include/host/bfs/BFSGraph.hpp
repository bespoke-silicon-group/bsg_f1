#pragma once
#include "HammerBlade.hpp"
#include "WGraph.hpp"
#include "bfs/graph.h"

namespace BFS {
    class BFSGraph {
        using WGraph = graph_tools::WGraph;
    public:
        BFSGraph(WGraph &g):
            _graph(std::move(g)),
            _hb(hammerblade::host::HammerBlade::Get()) {
        }
        BFSGraph(const WGraph &&g):
            _graph(std::move(g)),
            _hb(hammerblade::host::HammerBlade::Get()) {
        }

        const WGraph & graph() const { return _graph; }
        WGraph & graph() { return _graph; }
        const graph_t &kraph() const { return _kgraph; }
        kernel_graph_ptr_t        kgraph_dev() const { return _kgraph_dev; }
        kernel_vertex_data_ptr_t  edata_dev() const { return _edata_dev; }
        kernel_edge_data_ptr_t    vdata_dev() const { return _vdata_dev; }
        // format the graph on the device 
        void formatOnDevice();
        
    private:
        WGraph  _graph;
        graph_t _kgraph;
        kernel_graph_ptr_t _kgraph_dev;
        kernel_vertex_data_ptr_t _vdata_dev;
        kernel_edge_data_ptr_t   _edata_dev;
        hammerblade::host::HammerBlade::Ptr _hb;
    };
}
