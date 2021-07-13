// BlackParrot utilities header file
// Commonly used BlackParrot functions with modifications for
// the manycore platform

#ifndef BP_UTILS_H
#define BP_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <bsg_newlib_intf.h>
#include <bp_hb_platform.h>

#define BP_CFG_BASE_ADDR 0x00200000

/**************************** BlackParrot helper functions ****************************/

/*
 * Get hardware thread ID
 * @returns result of reading the mhartid register
 */
uint64_t bp_get_hart();

/*
 * Checks to see if a barrier is finished
 * @param[in] barrier_address --> An address in memory that all cores to write to after hitting the barrier
 * @param[in] total_num_cores --> Number of cores
 */
void bp_barrier_end(volatile uint64_t *barrier_address, uint64_t total_num_cores);

/*
 * Sends a hex digit to the host to print
 * @param[in] hex --> Hex digit to print
 */
void bp_hprint(uint8_t hex);

/*
 * Sends a character to the host to print
 * @param[in] ch --> Character to print
 */
void bp_cprint(uint8_t ch);

/*
 * Sends a finish packet to the host
 * @param[in] code --> Finish code
 */
void bp_finish(int16_t code);

#ifdef __cplusplus
}
#endif

#endif
