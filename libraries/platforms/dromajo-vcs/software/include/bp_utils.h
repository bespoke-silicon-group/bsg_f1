// BlackParrot utilities header file
// Note: This file is included here instead of the existing code
// in the perch library in order to allow for easy changes to the finish code
// in the future

#ifndef BP_UTILS_H
#define BP_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <bsg_newlib_intf.h>
#include <bsg_manycore.h>

// Memory mapped addresses to interact with the host
#define MC_ARGS_START_EPA_ADDR 0x0000
#define MC_ARGS_FINISH_EPA_ADDR 0x00FF
#define MC_CONFIG_START_EPA_ADDR 0x0100
#define MC_CONFIG_FINISH_EPA_ADDR 0x01FF
#define MC_RESET_DONE_EPA_ADDR 0x0200
#define MC_TX_VACANT_EPA_ADDR 0x0300
#define MC_FINISH_EPA_ADDR 0xEAD0
#define MC_TIME_EPA_ADDR 0xEAD4
#define MC_FAIL_EPA_ADDR 0xEAD8
#define MC_STDOUT_EPA_ADDR 0xEADC
#define MC_STDERR_EPA_ADDR 0xEAE0
#define MC_BRANCH_TRACE_EPA_ADDR 0xEAE4
#define MC_PRINT_STAT_EPA_ADDR 0xEA0C

#define MC_HOST_OP_FINISH_CODE 0xFFFFFFFF
#define DROMAJO_RW_FAIL_CODE 0xFFFFFFFF

// Memory mapped addresses to interact with the manycore bridge
// BlackParrot actually sets the MSB of its address to 1 to commnicate with the manycore
// Dromajo however is setup to identify the manycore as a device
#define MC_BASE_ADDR 0x500000
#define BP_TO_MC_REQ_FIFO_ADDR 0x1000
#define BP_TO_MC_REQ_CREDITS_ADDR 0x2000
#define MC_TO_BP_RESP_FIFO_ADDR 0x3000
#define MC_TO_BP_RESP_ENTRIES_ADDR 0x4000
#define MC_TO_BP_REQ_FIFO_ADDR 0x5000
#define MC_TO_BP_REQ_ENTRIES_ADDR 0x6000

#define BP_CFG_BASE_ADDR 0x00200000

// Manycore Bridge helper functions
/*
 * Reads the manycore bridge for number of credits used in the endpoint
 * @returns number of credits in the manycore bridge enddpoint
 */
int bp_hb_get_credits_used(int *credits_used);

/*
 * Writes a 128-bit manycore packet in 32-bit chunks to the manycore bridge FIFO
 * @param[in] pkt --> Pointer to the manycore packet
 */
int bp_hb_write_to_mc_bridge(hb_mc_packet_t *pkt);

/*
 * Checks if the MC to BP FIFO contains any entries
 * @param[in] type --> Type of FIFO to read from
 * @returns number of entries in the MC to BP FIFO
 */
int bp_hb_get_fifo_entries(int *entries, hb_mc_fifo_rx_t type);

/*
 * Reads the manycore bridge FIFOs in 32-bit chunks to form the 128-bit packet
 * @param[in] pkt --> Pointer to the manycore packet
 * @param[in] type --> Type of FIFO to read from
 * @returns HB_MC_SUCCESS on success, HB_MC_FAIL if FIFO type is unknown
 */
int bp_hb_read_from_mc_bridge(hb_mc_packet_t *pkt, hb_mc_fifo_rx_t type);

// BlackParrot helper functions
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
