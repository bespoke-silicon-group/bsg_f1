#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define MAX_UPDATES 512

int UPDATES = 0;
int X [MAX_UPDATES];

int prime(int *__restrict X_global, int n)
{
    int start = n * __bsg_tile_group_id;
    for (int i = 0; i < n; i++) {
        X[i] = X_global[start+i];
    }

    UPDATES = n;
    return 0;
}

int gups(int *__restrict A)
{
    bsg_cuda_print_stat_kernel_start();
    for (int i = 0; i < UPDATES; i++) {
        int x = X[i];
        int a = A[x]; // load
        A[x] = a ^ x; // store
    }
    bsg_cuda_print_stat_kernel_end();
    return 0;
}
