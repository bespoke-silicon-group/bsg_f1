#ifndef __CUDA_TESTS_H
#define __CUDA_TESTS_H

#ifdef COSIM

#include <utils/sh_dpi_tasks.h>
#include "fpga_pci_sv.h"
#include "bsg_manycore_driver.h"
#include "bsg_manycore_mem.h"
#include "bsg_manycore_loader.h"
#include "bsg_manycore_errno.h"	
#include "bsg_manycore_cuda.h"
#else // !COSIM

#include <bsg_manycore_driver.h>
#include <bsg_manycore_mem.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_cuda.h>
#endif // #ifdef COSIM

#include "../cl_manycore_regression.h"

#define __BSG_STRINGIFY(arg) #arg
#define BSG_STRINGIFY(arg) __BSG_STRINGIFY(arg)

#endif // __CUDA_TESTS_H
