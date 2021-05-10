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
#include <bsg_tile_group_barrier.h>
#include <string.h>
#include <queue>
#include <algorithm>
#include <array>
//#include <hello_world.hpp>
#include "inner_product.hpp"
#include "heap.hpp"
//#include "inner_product.h"

/* We wrap all external-facing C++ kernels with `extern "C"` to
 * prevent name mangling
 */

//#define V  1000000
#define VSIZE 100
#define NG 4
#define V_ENTRY 82026

#define EF        128
#define N_RESULTS 10

#define G_0 3
#define G_1 2
#define G_2 1
#define G_3 0

struct graph {
    const int *offsets;
    const int *neighbors;
    int V;
    int E;
};

using InnerProduct = InnerProductParallel_v1<BSG_TILE_GROUP_X_DIM, BSG_TILE_GROUP_Y_DIM>;

#ifdef __cplusplus
extern "C" {
#endif

//#define DEBUG_INPUT_TEST

// Uncomment to turn on debugging
//#define DEBUG_GREEDY_VCURR_TR
#define DEBUG_GREEDY_VIS_TR

    /**/
    int ipnsw_greedy_search (const graph *Gs,
                             bsg_attr_remote const float *__restrict database,
                             const float *query, int *seen,
                             int *v_curr_o, float *d_curr_o)
    {
    /* loc:2 */
        /**/
        float q[VSIZE];
        memcpy(q, query, sizeof(q));
        /* loc:2 */

        /* init code - can be hidden by library*/
        InnerProduct ip(database, q);
        ip.init();
        if (__bsg_id == 0) {
            bsg_saif_start();
            /**/
            int   v_curr = V_ENTRY;
            float d_curr = 0;

            d_curr = -1.0 * ip.inner_product(v_curr);

            /**/
            for (int i = 0; i < NG-1; i++) {
                struct graph G = Gs[i];
                bool changed = true;
                while (changed) {
                    changed = false;
                    /* loc:5 */
                    // fetch neighbors
                    /**/
                    for (int dst : G.neighbors(v_curr)) {
                        float d = -1.0 * ip.inner_product(dst);
                        if (d < d_curr) {
                            d_curr = d;
                            v_curr = dst;
                            changed = true;
                        }
                    }            
                }
            }
            /* loc: 10 */
            /**/
            *v_curr_o = v_curr;
            *d_curr_o = d_curr;
        }
        return 0;
    }
    /* loc: 5 */
    
#ifdef __cplusplus
}
#endif
