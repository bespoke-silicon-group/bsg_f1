#include "WGraph.hpp"
#include <cstring>
#include <cmath>

using namespace graph_tools;
using namespace std;

std::string GRAPH_TYPE;
int NUM_NODES;
int NUM_EDGES;
int NODE_SCALE;
std::string OUTPUT_NAME;

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s NUM_NODES NUM_EDGES OUTPUT_FILE\n", argv[0]);
        return 1;
    }

    GRAPH_TYPE = argv[1];
    NUM_NODES = atoi(argv[2]);
    NUM_EDGES = atoi(argv[3]);
    OUTPUT_NAME = argv[4];

    if (GRAPH_TYPE == "graph500") {
        NODE_SCALE = ceil(log2(NUM_NODES));
        Graph500Data::Generate(NODE_SCALE, NUM_EDGES).toFile(OUTPUT_NAME);
    } else if (GRAPH_TYPE == "uniform") {
        Graph500Data::Uniform(NUM_NODES, NUM_EDGES).toFile(OUTPUT_NAME);
    }
    
    return 0;
}
