//#define DEBUG
#include <bsg_manycore.h>

#ifdef DEBUG
#define BSG_TILE_GROUP_X_DIM 1 
#define BSG_TILE_GROUP_Y_DIM 1
#define bsg_tiles_X BSG_TILE_GROUP_X_DIM 
#define bsg_tiles_Y BSG_TILE_GROUP_Y_DIM 
#else
#include <bsg_set_tile_x_y.h>
#endif

#include <bsg_tile_group_barrier.hpp>
bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

#include <pr_nibble.hpp>
#include <cstring>

#ifdef DEBUG
#define pr_dbg(fmt, ...)            \
        bsg_printf(fmt, ##__VA_ARGS__)
#else
#define pr_dbg(fmt, ...)
#endif

__attribute__((section(".dram"))) float  * __restrict p;
__attribute__((section(".dram"))) float  * __restrict old_rank;
__attribute__((section(".dram"))) float  * __restrict new_rank;
__attribute__((section(".dram"))) int  * __restrict out_degree;


template <typename APPLY_FUNC > int edgeset_apply_pull_parallel_from_vertexset(int *in_indices , int *in_neighbors, int* from_vertexset, APPLY_FUNC apply_func, int V, int E, int block_size_x) 
{
    int start, end;
    local_range(V, &start, &end);
    for ( int d = start; d < end; d++) {
        int degree = in_indices[d + 1] - in_indices[d];
        int * neighbors = &in_neighbors[in_indices[d]];
        for(int s = 0; s < degree; s++) { 
            if(from_vertexset[neighbors[s]]) {
                apply_func (neighbors[s] , d);
            }
        } //end of loop on in neighbors
    } //end of outer for loop
    return 0;
} //end of edgeset apply function 

template <typename APPLY_FUNC > int edgeset_apply_push_parallel_from_vertexset(int *out_indices , int *out_neighbors, int* from_vertexset, APPLY_FUNC apply_func, int V, int E, int block_size_x) 
{
    int start, end;
    local_range(V, &start, &end);
    for ( int s = start; s < end; s++) {
        if(from_vertexset[s]) {
            int degree = out_indices[s + 1] - out_indices[s];
            int * neighbors = &out_neighbors[out_indices[s]];
            for(int d = 0; d < degree; d++) { 
                apply_func (s, neighbors[d]);
    
            }
        } //end of loop on in neighbors
    } //end of outer for loop
    return 0;
} //end of edgeset apply function 

struct updateEdge
{
    void operator() (int src, int dst)
    {
        float alpha = 0.15; 
        new_rank[dst] = (new_rank[dst] + (((((1)    - alpha) / ((1)  + alpha)) * old_rank[src]) / out_degree[src]));
    };
};
struct updateSelf
{
    void operator() (int v)
    {
        float alpha = 0.15; 
        p[v] = (p[v] + ((((2)  * alpha) / ((1)  + alpha)) * old_rank[v]));
        new_rank[v] = (0) ;
    };
};
struct filter_frontier
{
    bool operator() (int v)
    {
        float epsilon = (float) 1e-6; 
        bool output ;
        if(new_rank[v] == 0) return 0;
        output = (new_rank[v]) > ((out_degree[v] * epsilon));
        return output;
    };
};

extern "C" int  __attribute__ ((noinline)) updateSelf_kernel(int * frontier, int V, int tag_c) {
    bsg_cuda_print_stat_start(tag_c);
    barrier.sync();
    int start, end;
    local_range(V, &start, &end);
    for (int iter_x = start; iter_x < end; iter_x++) {
        if(frontier[iter_x]) {updateSelf()(iter_x);}
    }
    bsg_cuda_print_stat_end(tag_c);
    barrier.sync();
    return 0;
}
extern "C" int __attribute__ ((noinline)) edgeset_apply_pull_parallel_from_vertexset_call(int *in_indices, int *in_neighbors, int *frontier, int V, int E, int block_size_x, int tag_c) {
    barrier.sync();
    bsg_cuda_print_stat_start(tag_c);
    bsg_saif_start();
    edgeset_apply_pull_parallel_from_vertexset(in_indices, in_neighbors, frontier, updateEdge(), V, E, block_size_x);
    bsg_saif_end();
    bsg_cuda_print_stat_end(tag_c);
    barrier.sync();
    return 0;
}

 extern "C" int __attribute__ ((noinline)) edgeset_apply_push_parallel_from_vertexset_call(int *out_indices, int *out_neighbors, int *frontier, int V, int E, int block_size_x, int tag_c) {
    barrier.sync(); 
    bsg_cuda_print_stat_start(tag_c);
    bsg_saif_start();
    edgeset_apply_push_parallel_from_vertexset(out_indices, out_neighbors, frontier, updateEdge(), V, E, block_size_x);
    bsg_saif_end();
    bsg_cuda_print_stat_end(tag_c);
    barrier.sync();
    return 0;
}

extern "C" int __attribute__ ((noinline)) filter_frontier_where_call(int * next5, int V, int block_size_x, int tag_c) { 
    bsg_cuda_print_stat_start(tag_c);
    barrier.sync();
    int start, end;
    local_range(V, &start, &end);
    for (int iter_x = start; iter_x < end; iter_x++) {
        if (iter_x < V) {
            next5[iter_x] = 0;
            if ( filter_frontier()( iter_x ) ) {
                next5[iter_x] = 1;
            }
                }
        else { break; }
    } //end of loop
    bsg_cuda_print_stat_end(tag_c);
    barrier.sync();
    return 0;
}

extern "C" void prefetch(int * in_indices, int * in_neighbors, int * from_vertexset, int V, int E) {
        int id = __bsg_id;
        int threads = bsg_tiles_X * bsg_tiles_Y;
        // prefetch all data;
        for (int i = 32 * id; i < E; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (in_neighbors[i]));
        }
        for (int i = 32 * id; i < V; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (in_indices[i]));
        }
        for (int i = 32 * id; i < V; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (from_vertexset[i]));
        }
        for (int i = 32 * id; i < V; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (out_degree[i]));
        }
        for (int i = 32 * id; i < V; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (p[i]));
        }
        for (int i = 32 * id; i < V; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (old_rank[i]));
        }
        for (int i = 32 * id; i < V; i += 32 * threads) {
                asm volatile ("lw x0, %[p]" :: [p] "m" (new_rank[i]));
        }
        barrier.sync();
        return ;

}
