#ifndef __spmd_TESTS_H
#define __spmd_TESTS_H

#ifdef COSIM

#include <utils/sh_dpi_tasks.h>
#include "fpga_pci_sv.h"
#include "bsg_manycore_driver.h"
#include "bsg_manycore_mem.h"
#include "bsg_manycore_loader.h"
#include "bsg_manycore_errno.h"	

#else // !COSIM

#include <bsg_manycore_driver.h>
#include <bsg_manycore_mem.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_errno.h>

#endif // #ifdef COSIM

#define __BSG_STRINGIFY(arg) #arg
#define BSG_STRINGIFY(arg) __BSG_STRINGIFY(arg)

#endif // __spmd_TESTS_H
