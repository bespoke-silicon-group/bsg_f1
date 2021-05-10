/*
 * This kernel prints the Hello World message
 */

// BSG_TILE_GROUP_X_DIM and BSG_TILE_GROUP_Y_DIM must be defined
// before bsg_manycore.h and bsg_tile_group_barrier.h are
// included. bsg_tiles_X and bsg_tiles_Y must also be defined for
// legacy reasons, but they are deprecated.
#define BSG_TILE_GROUP_X_DIM 4
#define BSG_TILE_GROUP_Y_DIM 4
#define bsg_tiles_X BSG_TILE_GROUP_X_DIM
#define bsg_tiles_Y BSG_TILE_GROUP_Y_DIM
#include <bsg_manycore.hpp>
#include <bsg_tile_group_barrier.h>
#include <string.h>
#include <queue>
#include <algorithm>
#include <array>
//#include <hello_world.hpp>
#include "inner_product.hpp"
#include "heap.hpp"
#include "set.hpp"
//#include "inner_product.h"

/* We wrap all external-facing C++ kernels with `extern "C"` to
 * prevent name mangling
 */

#define N_V  1000000
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

class LT {
public:
    bool operator()(const std::pair<float, int> &lhs, const std::pair<float, int> &rhs) {
        return  std::get<0>(lhs) < std::get<0>(rhs);
    }
};

class GT {
public:
    bool operator()(const std::pair<float, int> &lhs, const std::pair<float, int> &rhs) {
        return std::get<0>(lhs) > std::get<0>(rhs);
    }
};

#ifdef __cplusplus
extern "C" {
#endif

// Uncomment to turn on debugging
#define DEBUG_BEAM_SEARCH_TRAVERSED_TRACE
//#define DEBUG_BEAM_SEARCH_INPUT

    using InnerProduct = InnerProductParallel_Y<BSG_TILE_GROUP_Y_DIM>;

    static constexpr int SYNC_INV  = -1;
    static constexpr int SYNC_DONE = -2;

    void ipnsw_x_master(bsg_attr_remote const float *__restrict database,
                             const float *query,
                             int   *dst_p,
                             float *distance_p,
                        DenseSet_v1<int> *seen,
                        InnerProduct *ip_y)
    {
        float *result = bsg_tile_group_remote_pointer<float>(0, 0, &distance_p[__bsg_x]);
        while (true) {
            int dst = sleep_until_valid(dst_p, SYNC_INV);
            if (dst == SYNC_DONE)
                break;

            if (!seen->in(dst)) {
                seen->atomic_insert(dst);
                //bsg_print_int(dst);
                *result  = -1.0 * ip_y->inner_product(dst);
            } else {
                *result = -INFINITY;
            }
        }
    }

    int ipnsw_beam_search(const graph *Gs,
                          bsg_attr_remote const float *__restrict database,
                          const float *query,
                          int *seen_mem,
                          int *v_curr_o, float *d_curr_o,
                          std::pair<float, int> *candidates_mem,
                          std::pair<float, int> *results_mem,
                          int *n_results)
    {
        // keep track of vertices seen
        DenseSet_v1<int>seen(seen_mem);

        // fetch graph and q out of memory
        struct graph G = Gs[G_0];
        float q[VSIZE];
        bsg_cuda_print_stat_start(0);        
        memcpy(q, query, sizeof(q));

        InnerProduct ip_y(database, q);
        ip_y.init();

        int   dst_slave = SYNC_INV;
        float dist_result[BSG_TILE_GROUP_X_DIM];
        
        if (__bsg_y == 0) {        
            if (__bsg_x == 0) {
                
                int *dst_slave_ptr[BSG_TILE_GROUP_X_DIM];
                for (int x = 0; x < BSG_TILE_GROUP_X_DIM; ++x) {
                    dst_slave_ptr[x] = bsg_tile_group_remote_pointer(x, 0, &dst_slave);
                    dist_result[x] = INFINITY;
                }

                // retrieve results from greedy walk
                int v_curr   = *v_curr_o;
                float d_curr = *d_curr_o;
#ifdef DEBUG_BEAM_SEARCH_INPUT
                bsg_print_int(v_curr);
                bsg_print_float(d_curr);
#endif

                // initialize priority queues
                DynHeap<std::pair<float, int>, GT> candidates(candidates_mem, 512);
                DynHeap<std::pair<float, int>, LT> results(results_mem, 128);

                candidates.push({d_curr, v_curr});
                results.push({d_curr, v_curr});

                float d_worst = d_curr;
                seen.insert(v_curr);

                while (!candidates.empty()) {
                    int   v_best;
                    float d_best;

                    auto best = candidates.pop();
                    v_best = std::get<1>(best);
                    d_best = std::get<0>(best);

                    d_worst = std::get<0>(results.top());
#ifdef DEBUG_BEAM_SEARCH_TRAVERSED_TRACE
                    bsg_print_int(-v_best);
#endif

                    if (d_best > d_worst) {
                        break;
                    }

                    // traverse neighbors of v_best
                    int dst_0 = G.offsets[v_best];
                    int degree = v_curr == G.V-1 ? G.E - dst_0 : G.offsets[v_best+1] - dst_0;

                    // traverse neighbors
                    for (int dst_i = 0;
                         dst_i < degree;
                         dst_i += BSG_TILE_GROUP_X_DIM) {
                        // read-in work
                        int dst_n = std::min(BSG_TILE_GROUP_X_DIM, degree-dst_i);
                        int dst_v[dst_n];
                        memcpy(dst_v, &G.neighbors[dst_0+dst_i], sizeof(dst_v));

                        // delegate work
                        int dst;
                        for (int dst_j = 1; dst_j < dst_n; ++dst_j) {
                            dst = dst_v[dst_j];
                            *dst_slave_ptr[dst_j] = dst;                        
                        }
                        // work myself 
                        {
                            dst = dst_v[0];
                            if (!seen.in(dst)) {
                                seen.atomic_insert(dst);                            
                                dist_result[0] = -1.0 * ip_y.inner_product(dst);
                            } else {
                                dist_result[0] = -INFINITY;
                            }
                        }
                        // reduce
                        for (int dst_j = 0; dst_j < dst_n; ++dst_j) {
                            dst = dst_v[dst_j];

#ifdef DEBUG_BEAM_SEARCH_TRAVERSED_TRACE
                            bsg_print_int(dst);
#endif                        
                            float d_neib = sleep_until_valid(&dist_result[dst_j], INFINITY);
#ifdef DEBUG_BEAM_SEARCH_TRAVERSED_TRACE
                            bsg_print_float(d_neib);
#endif
                            // already seen?
                            if (d_neib == -INFINITY)
                                continue;

                            d_worst = std::get<0>(results.top());
                            // if there's room for new result or this distance is promising
                            if ((results.size() < EF) || (d_neib < d_worst)) {
                            
                                // push onto candidates and results
                                candidates.push({d_neib, dst});
                                results.push({d_neib, dst});
                            
                                // prune down to recall
                                if (results.size() > EF)
                                    results.pop();
                            }
                        }                    
                    
                    }

                }

                // signal all columns done
                for (int tile = 0; tile < BSG_TILE_GROUP_X_DIM; ++tile)
                    *dst_slave_ptr[tile] = SYNC_DONE;

                int n_res = std::min(results.size(), N_RESULTS);
                std::sort(results_mem, results_mem+results.size(), LT());
                *n_results = n_res;
        
            } else { // bsg_x != 0
                ipnsw_x_master(database, q, &dst_slave, dist_result, &seen, &ip_y);
            }
        }

        ip_y.exit();
        bsg_cuda_print_stat_end(0);
        return 0;
    }

#ifdef __cplusplus
}
#endif
