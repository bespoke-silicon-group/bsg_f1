#pragma once
#include <HammerBlade.hpp>
#include <Graph.hpp>
#include <Graph500Data.hpp>
#include <vector>

namespace ipnsw {
    class Graph {
    public:
        Graph() : Graph(graph_tools::Graph()) {}
        Graph(const graph_tools::Graph &g) : _graph(g) {}
        Graph(graph_tools::Graph &&g) : _graph(g) {}

        void initialize_on_device() {
            using hammerblade::host::HammerBlade;
            HammerBlade::Ptr hb = HammerBlade::Get();

            auto & offsets = _graph.get_offsets();
            auto & neighbors = _graph.get_neighbors();
            
            _offsets  = hb->alloc(offsets.size() * sizeof(offsets[0]));
            _neighbors = hb->alloc(neighbors.size() * sizeof(neighbors[0]));

            hb->push_write(_offsets,   &offsets[0],   offsets.size() * sizeof(offsets[0]));
            hb->push_write(_neighbors, &neighbors[0], neighbors.size() * sizeof(neighbors[0]));
        }

        graph_tools::Graph & graph() { return _graph; }
        const graph_tools::Graph & graph() const { return _graph; }
        hb_mc_eva_t offsets() const { return _offsets; }
        hb_mc_eva_t neighbors() const  { return _neighbors; }
        
        static hb_mc_eva_t InitializeMetadataOnDevice(const std::vector<Graph> & Gs) {
            using hammerblade::host::HammerBlade;
            HammerBlade::Ptr hb = HammerBlade::Get();            
            struct metadata {
                hb_mc_eva_t offset;
                hb_mc_eva_t neighbors;
                int V;
                int E;
            };

            std::vector<metadata> metad;
            for (auto & g : Gs) {
                std::cout << "Host: offset = " << std::hex << g.offsets() << " neighbors = " << g.neighbors() << std::endl;
                std::cout << std::dec;
                metadata m = {
                    .offset = g.offsets(),
                    .neighbors = g.neighbors(),
                    g.graph().num_nodes(),
                    g.graph().num_edges()
                };
                metad.push_back(m);
            }
            
            hb_mc_eva_t metadata = hb->alloc(sizeof(struct metadata) * metad.size());
            hb->push_write(metadata, &metad[0], sizeof(struct metadata) * metad.size());
            hb->sync_write();

            return metadata;
        }
        
    private:
        graph_tools::Graph _graph;

        hb_mc_eva_t _offsets;
        hb_mc_eva_t _neighbors;
    };
}
