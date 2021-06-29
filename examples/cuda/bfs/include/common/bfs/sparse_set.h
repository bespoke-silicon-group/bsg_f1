#pragma once
#include "bfs/types.h"

typedef struct sparse_set {
    int              set_size;
    int              set_cap;
    int              members[1];
} sparse_set_t;

#ifdef __KERNEL__
typedef sparse_set_t*     kernel_sparse_set_ptr_t;
#else
typedef kernel_void_ptr_t kernel_sparse_set_ptr_t;
#endif
