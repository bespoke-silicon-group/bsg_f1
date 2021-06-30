#include <string>
#include "WGraph.hpp"
#include "SparsePushBFS.hpp"
using namespace std;
using namespace graph_tools;
std::string INPUT_NAME;
std::string OUTPUT_NAME;
std::string ROOT;
int ITERS;

int main(int argc, char *argv[])
{
    int root;

    if (argc < 5) {
        fprintf(stderr, "usage: %s INPUT_NAME ROOT ITERS OUTPUT_NAME\n", argv[0]);
        fprintf(stderr, "    ROOT    avg|max|[number]\n");
    }

    INPUT_NAME = argv[1];
    ROOT = argv[2];
    ITERS = atoi(argv[3]);
    OUTPUT_NAME = argv[4];
    
    WGraph wg = WGraph::FromGraph500Data(Graph500Data::FromFile(INPUT_NAME));

    if (ROOT == "avg") {
        root = static_cast<int>(wg.node_with_avg_degree());
    } else if (ROOT == "max") {
        root = static_cast<int>(wg.node_with_max_degree());
    } else {
        root = atoi(ROOT.c_str());
    }

    std::vector<SparsePushBFS> stats = SparsePushBFS::RunBFS(wg, root, ITERS);

    // write the output
    FILE *output = fopen(OUTPUT_NAME.c_str(), "w");
    if (output == NULL) {
        fprintf(stderr, "failed to open output file '%s': %m\n", OUTPUT_NAME.c_str());
        return 1;
    }

    // header
    #define FIELD_STR_FMT "s"
    #define FIELD_INT_FMT "d"

    fprintf(output,
            "%"  FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            ",%" FIELD_STR_FMT
            "\n",
            "input_name",
            "root",
            "num_nodes",
            "num_edges",
            "iteration",
            "frontier_in_size",
            "frontier_out_size",
            "visited_in_size",
            "visited_out_size",
            "traversed_edges",
            "updates"
        );

    for (int i = 0; i < stats.size(); i++) {
        fprintf(output,
                "%"  FIELD_STR_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT                
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                ",%" FIELD_INT_FMT
                "\n",
                INPUT_NAME.c_str(),
                root,
                static_cast<int>(wg.num_nodes()),
                static_cast<int>(wg.num_edges()),
                i,
                stats[i].frontier_in().size(),
                stats[i].frontier_out().size(),
                stats[i].visited_in().size(),
                stats[i].visited_out().size(),
                stats[i].traversed_edges(),
                stats[i].updates());
    }

    fclose(output);
    
    return 0;
}
