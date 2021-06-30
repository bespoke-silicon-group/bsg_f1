#include "WGraph.hpp"
#include <cstring>
#include <cmath>

using namespace graph_tools;
using namespace std;

int NUM_NODES;
int NUM_EDGES;
int NODE_SCALE;
std::string OUTPUT_NAME;

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s NUM_NODES NUM_EDGES OUTPUT_FILE\n", argv[0]);
        return 1;
    }
    
    NUM_NODES = atoi(argv[1]);
    NUM_EDGES = atoi(argv[2]);
    OUTPUT_NAME = argv[3];
    
    NODE_SCALE = ceil(log2(NUM_NODES));

    Graph500Data::Generate(NODE_SCALE, NUM_EDGES).toFile(OUTPUT_NAME);
    
    return 0;
}
