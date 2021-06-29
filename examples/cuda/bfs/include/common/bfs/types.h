#ifndef COMMON_TYPES
#define COMMON_TYPES
#ifdef __KERNEL__
#include "stdint.h"
typedef int*   kernel_int_ptr_t;
typedef float* kernel_float_ptr_t;
typedef void*  kernel_void_ptr_t;
#else
#include "bsg_manycore_cuda.h"
typedef hb_mc_eva_t kernel_int_ptr_t;
typedef hb_mc_eva_t kernel_float_ptr_t;
typedef hb_mc_eva_t kernel_void_ptr_t;
#endif
#endif
