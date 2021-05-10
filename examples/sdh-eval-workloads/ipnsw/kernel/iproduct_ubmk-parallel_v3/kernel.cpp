/*
 * This kernel prints the Hello World message
 */

// BSG_TILE_GROUP_X_DIM and BSG_TILE_GROUP_Y_DIM must be defined
// before bsg_manycore.h and bsg_tile_group_barrier.h are
// included. bsg_tiles_X and bsg_tiles_Y must also be defined for
// legacy reasons, but they are deprecated.
#define BSG_TILE_GROUP_X_DIM 2
#define BSG_TILE_GROUP_Y_DIM 2
#define bsg_tiles_X BSG_TILE_GROUP_X_DIM
#define bsg_tiles_Y BSG_TILE_GROUP_Y_DIM
#include <bsg_manycore.h>
//#include <bsg_tile_group_barrier.h>
#include <bsg_tile_group_barrier.hpp>
#include <string.h>
#include <queue>
#include <algorithm>
#include <array>
#include <atomic>
//#include <hello_world.hpp>
#include "inner_product.hpp"
#include "heap.hpp"
//#include "inner_product.h"
#include "sleep_until_valid.hpp"

//#define V  1000000
#define VSIZE 100
#define NG 4
#define V_ENTRY 82026

#define EF        128
#define N_RESULTS 10

#define VISIT_BUFSIZE 512

using InnerProduct = InnerProductParallel_v1<BSG_TILE_GROUP_X_DIM, BSG_TILE_GROUP_Y_DIM>;
using barrier = bsg_barrier<BSG_TILE_GROUP_X_DIM, BSG_TILE_GROUP_Y_DIM>;

#ifdef __cplusplus
extern "C" {
#endif

    int inner_product_ubmk(bsg_attr_remote const float * __restrict database,
                           const float * __restrict query,
                           int N,
                           int *visit_remote_all)
    {
        float q[VSIZE];
        memcpy(q, query, sizeof(q));
        barrier b;

        bsg_cuda_print_stat_start(0);
        
        InnerProduct ip(database, q);
        ip.init();        
        float r = 0.0;
        int visit[VISIT_BUFSIZE];
        int *visit_remote = &visit_remote_all[N * __bsg_tile_group_id];

        for (int i = 0; i < N; i += VISIT_BUFSIZE) {
            size_t sz = std::min(VISIT_BUFSIZE, (N-i));
            memcpy(visit, &visit_remote[i], sz*sizeof(int));

            for (int j = 0; j < sz; ++j) {
                // read k
                int k = visit[j];                
                float rp = ip.inner_product(k);
                r += rp;
            }
        }

        ip.exit();
        bsg_cuda_print_stat_end(0);
        b.sync();
        return (int)(r);
    }
#ifdef __cplusplus
}
#endif
