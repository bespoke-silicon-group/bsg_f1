#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define MAX_UPDATES 512

#ifndef CONCURRENCY
#define CONCURRENCY 1
#endif

int UPDATES = 0;
int X [MAX_UPDATES];

int prime(bsg_attr_remote int *__restrict X_global, int n)
{
    int start = n * __bsg_tile_group_id;
    for (int i = 0; i < n; i++) {
        X[i] = X_global[start+i];
    }

    UPDATES = n;
    return 0;
}

int gups(bsg_attr_remote int *__restrict A)
{
    bsg_cuda_print_stat_kernel_start();
    for (int i = 0; i < UPDATES; i += CONCURRENCY) {
        int x[CONCURRENCY];
        int a[CONCURRENCY];

        bsg_unroll(32)
        for (int j = 0; j < CONCURRENCY; j++) {
            x[j] = X[i+j];
            a[j] = A[x[j]]; // load
        }

        bsg_unroll(32)
        for (int j = 0; j < CONCURRENCY; j++) {
            A[x[j]] = a[j] ^ x[j]; // store
        }
    }
    bsg_cuda_print_stat_kernel_end();
    return 0;
}
