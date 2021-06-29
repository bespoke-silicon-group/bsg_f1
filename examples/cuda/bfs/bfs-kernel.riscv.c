#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bfs/graph.h"
#include "bfs/sparse_set.h"

int bfs(graph_t *G_ptr,
        bsg_attr_remote sparse_set_t *frontier_in,
        bsg_attr_remote int *frontier_out,
        bsg_attr_remote int *visited)
{
    bsg_cuda_print_stat_kernel_start();
    graph_t G = *G_ptr;
    for (int src_i = __bsg_tile_group_id;
         src_i < frontier_in->set_size;
         src_i += __bsg_grid_dim_x*__bsg_grid_dim_y) {
        // update all neibs
        int src = frontier_in->members[src_i];
        vertex_data_t *src_data = &G.vertex_data[src];
        int degree = src_data->degree;
        int *neib = src_data->neib;
        for (int dst_i = 0; dst_i < degree; dst_i++) {
            int dst = neib[dst_i];
            if (visited[dst] == 0) {
                frontier_out[dst] = 1;
                visited[dst] = 1;
            }
        }
    }
    bsg_cuda_print_stat_kernel_end();
    return 0;
}
