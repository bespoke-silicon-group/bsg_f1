#ifndef __TEST_MANYCORE_PARAMETERS_H
#define __TEST_MANYCORE_PARAMETERS_H

// These are explicitly not copied from the bsg_manycore_eva.h 
#define LOCAL_EPA_SIZE (1<<18)
#define DMEM_EPA_SIZE (1<<12)
#define DMEM_EPA_OFFSET (1<<12)
#define DMEM_EVA_OFFSET (1<<12)

#define GROUP_INDICATOR (1 << 29)
#define GROUP_EPA_SIZE DMEM_EPA_SIZE
#define GROUP_X_BITS 6
#define GROUP_X_MAX ((1 << GROUP_X_BITS) - 1)
#define GROUP_X_MASK GROUP_X_MAX
#define GROUP_X_OFFSET 18

#define GROUP_Y_BITS 5
#define GROUP_Y_MAX ((1 << GROUP_Y_BITS) - 1)
#define GROUP_Y_MASK GROUP_Y_MAX
#define GROUP_Y_OFFSET (GROUP_X_OFFSET + GROUP_X_BITS)

#define GLOBAL_INDICATOR_WIDTH 1
#define GLOBAL_INDICATOR (1 << 30)
#define GLOBAL_EPA_SIZE DMEM_EPA_SIZE
#define GLOBAL_X_BITS 6
#define GLOBAL_X_MAX ((1 << GLOBAL_X_BITS) - 1)
#define GLOBAL_X_MASK GLOBAL_X_MAX
#define GLOBAL_X_OFFSET 18

#define GLOBAL_Y_BITS 6
#define GLOBAL_Y_MAX ((1 << GLOBAL_Y_BITS) - 1)
#define GLOBAL_Y_MASK GLOBAL_Y_MAX
#define GLOBAL_Y_OFFSET (GLOBAL_X_OFFSET + GLOBAL_X_BITS)

#define DRAM_INDICATOR_WIDTH 1
#define DRAM_INDICATOR (1 << 31)
#define DRAM_EPA_SIZE 16384

#endif // __TEST_MANYCORE_PARAMETERS_H
