/*
 * This kernel prints the Hello World message
 */

// BSG_TILE_GROUP_X_DIM and BSG_TILE_GROUP_Y_DIM must be defined
// before bsg_manycore.h and bsg_tile_group_barrier.h are
// included. bsg_tiles_X and bsg_tiles_Y must also be defined for
// legacy reasons, but they are deprecated.
#define BSG_TILE_GROUP_X_DIM 1
#define BSG_TILE_GROUP_Y_DIM 1
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

#ifdef __cplusplus
extern "C" {
#endif

    int inner_product_ubmk(const float *database, const float *query, int N)
    {
        float q[VSIZE];
        float r = 0;

        bsg_print_int(N);
        memcpy(q, query, sizeof(q));

        bsg_cuda_print_stat_start(0);
        // perform a random inner product N times
        for (int i = 0; i < N; ++i) {
            const float *b = &database[i*3*VSIZE];
            r += inner_product<BSG_TILE_GROUP_X_DIM, BSG_TILE_GROUP_Y_DIM>(q,b);
        }
        bsg_cuda_print_stat_end(0);
        return (int)(r);
    }
#ifdef __cplusplus
}
#endif
