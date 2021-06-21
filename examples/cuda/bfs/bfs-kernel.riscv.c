#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "graph.h"

int bfs(graph_t *G_ptr)
{
    graph_t G = *G_ptr;
    for (int src = 0; src < G.V; src++) {
        int *neib  = G.vertex_data[src].neib;
        int degree = G.vertex_data[src].degree;
        bsg_print_int(-src);
        for (int dst_i = 0; dst_i < degree; dst_i++) {
            bsg_print_int(neib[dst_i]);
        }
    }
    return 0;
}
