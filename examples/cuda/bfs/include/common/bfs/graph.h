#ifndef COMMON_GRAPH
#define COMMON_GRAPH
#ifdef __cplusplus
extern "C" {
#endif
#include "bfs/types.h"

typedef int edge_data_t;
#ifdef __KERNEL__
typedef bsg_attr_remote edge_data_t* kernel_edge_data_ptr_t;
#else
typedef kernel_void_ptr_t kernel_edge_data_ptr_t;
#endif

typedef struct vertex_data {
    kernel_edge_data_ptr_t neib;
    int degree;
} vertex_data_t;

#ifdef __KERNEL__
    typedef bsg_attr_remote vertex_data_t*    kernel_vertex_data_ptr_t;
#else
    typedef kernel_void_ptr_t kernel_vertex_data_ptr_t;
#endif
    
typedef struct graph {
    int V;
    int E;
    kernel_vertex_data_ptr_t vertex_data;
    kernel_edge_data_ptr_t   edge_data;
} graph_t;
    

#ifdef __KERNEL__
    typedef graph_t* kernel_graph_ptr_t;
#else
    typedef kernel_void_ptr_t kernel_graph_ptr_t;
#endif
    
#ifdef __cplusplus
}
#endif
#endif
