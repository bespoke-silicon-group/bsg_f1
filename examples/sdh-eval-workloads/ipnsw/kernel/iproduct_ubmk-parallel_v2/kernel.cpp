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

#ifdef __cplusplus
extern "C" {
#endif

//#define DEBUG_MASTER
//#define DEBUG_SLAVE
#define iproduct(x,y)                                                   \
    inner_product_parallel_v1<BSG_TILE_GROUP_X_DIM, BSG_TILE_GROUP_Y_DIM>(x,y)

    #define SYNC_INV  -2
    #define SYNC_DONE -1

        __attribute__((noinline))
    int inner_product_ubmk_master(bsg_attr_remote const float * __restrict database,
                                  const float * __restrict query,
                                  int N,
                                  int *visit_remote_all,
                                  int   *kp,
                                  float *rp)
    {
        float r = 0.0;
        int visit[VISIT_BUFSIZE];
        int *visit_remote = &visit_remote_all[N * __bsg_tile_group_id];

        // pre-compute addresses on remote tiles
        int   *kp_group[BSG_TILE_GROUP_X_DIM * BSG_TILE_GROUP_Y_DIM];

        for (int tile_x = 0; tile_x < BSG_TILE_GROUP_X_DIM; ++tile_x) {
            for (int tile_y = 0; tile_y < BSG_TILE_GROUP_Y_DIM; ++tile_y) {                
                kp_group[bsg_x_y_to_id(tile_x, tile_y)] = bsg_tile_group_remote_pointer(tile_x, tile_y, kp);
            }
        }

        for (int i = 0; i < N; i += VISIT_BUFSIZE) {
            size_t sz = std::min(VISIT_BUFSIZE, (N-i));
            memcpy(visit, &visit_remote[i], sz*sizeof(int));

            for (int j = 0; j < sz; ++j) {
                // read k
                int k = visit[j];

                // set k on all tiles
                for (int tile = 0; tile < BSG_TILE_GROUP_X_DIM*BSG_TILE_GROUP_Y_DIM; ++tile)
                    *kp_group[tile] = k;

                float r_local = iproduct(query, &database[k * VSIZE]);
#ifdef DEBUG_MASTER
                bsg_print_float(r_local);
#endif
                rp[__bsg_id] = r_local;

                // read r from all tiles
                for (int tile = 0; tile < BSG_TILE_GROUP_X_DIM*BSG_TILE_GROUP_Y_DIM; ++tile) {
                    float r_remote = sleep_until_valid(&rp[tile], INFINITY);
#ifdef DEBUG_MASTER
                    bsg_print_float(r_remote);
#endif
                    r += r_remote;
                }
            }
        }

        for (int tile = 0; tile < BSG_TILE_GROUP_X_DIM*BSG_TILE_GROUP_Y_DIM; ++tile)
            *kp_group[tile] = SYNC_DONE;

        return (int)r;
    }

    __attribute__((noinline))
    void inner_product_ubmk_slave(bsg_attr_remote const float * __restrict database,
                                  const float * __restrict query,
                                  int   *kp,
                                  float *rp)
    {
        float r = 0.0;
        int k;

        while (true) {
            // load next
            k = sleep_until_valid(kp, SYNC_INV);
            if (k == SYNC_DONE)
                break;

            r = iproduct(query, &database[k * VSIZE]);
            *rp = r;
#ifdef DEBUG_SLAVE
            bsg_print_float(r);
#endif
        }
    }
    
    int inner_product_ubmk(bsg_attr_remote const float * __restrict database,
                           const float * __restrict query,
                           int N,
                           int *visit_remote_all)
    {
        static int   k = SYNC_INV;
        static float r [BSG_TILE_GROUP_X_DIM * BSG_TILE_GROUP_Y_DIM] = {INFINITY};
        float rr;

        float q[VSIZE];
        memcpy(q, query, sizeof(q));

        bsg_cuda_print_stat_start(0);
        if (__bsg_id == 0) {
            // enter master loop
            rr = inner_product_ubmk_master(database, q, N, visit_remote_all, &k, r);
        } else {
            // enter slave loop
            inner_product_ubmk_slave(database, q, &k, bsg_tile_group_remote_pointer(0,0, &r[__bsg_id]));
        }
        bsg_cuda_print_stat_end(0);
        
        return (int)(rr);
    }
#ifdef __cplusplus
}
#endif
