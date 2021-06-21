#include "BFSGraph.hpp"
#include "graph.h"

namespace BFS {
    void BFSGraph::formatOnDevice() {        
        // allocate edges
        _edata_dev = _hb->alloc(graph().num_edges() * sizeof(edge_data_t));
        // copy edges over
        _hb->push_write(_edata_dev, &(graph().get_neighbors()[0]), graph().num_edges() * sizeof(edge_data_t));

        // format vertex data in terms of edge data (aka offsets -> pointers)
        std::vector<vertex_data_t> vdata(graph().num_nodes());
        for (int v = 0; v < graph().num_nodes(); v++) {
            vdata[v].neib = _edata_dev + graph().offset(v) * sizeof(edge_data_t);
            vdata[v].degree = graph().degree(v);
        }

        // allocate vertex data + push write
        _vdata_dev = _hb->alloc(graph().num_nodes() * sizeof(vertex_data_t));
        _hb->push_write(_vdata_dev, &vdata[0], vdata.size() * sizeof(vertex_data_t));

        // format graph meta data
        graph_t g;
        g.V = graph().num_nodes();
        g.E = graph().num_edges();
        g.vertex_data = _vdata_dev;
        g.edge_data   = _edata_dev;

        // allocate + push write graph meta data
        _kgraph_dev = _hb->alloc(sizeof(g));
        _hb->push_write(_kgraph_dev, &g, sizeof(g));
    }
}
