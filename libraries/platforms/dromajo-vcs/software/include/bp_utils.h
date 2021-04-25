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
#include "bsg_newlib_intf.h"
#include <bsg_manycore.h>

// Memory mapped addresses to interact with the host
#define HOST_DEV_BASE_ADDR ((char *)(0x00100000))
#define GETCHAR_BASE_ADDR  ((char *)(HOST_DEV_BASE_ADDR+0x0000))
#define PUTCHAR_BASE_ADDR  ((char *)(HOST_DEV_BASE_ADDR+0x1000))
#define FINISH_BASE_ADDR   ((char *)(HOST_DEV_BASE_ADDR+0x2000))

// Memory mapped addresses to interact with the manycore bridge
#define MC_BASE_ADDR 0x500000
#define BP_TO_MC_REQ_FIFO_ADDR 0x1000
#define BP_TO_MC_REQ_CREDITS_ADDR 0x2000
#define MC_TO_BP_RESP_FIFO_ADDR 0x3000
#define MC_TO_BP_RESP_ENTRIES_ADDR 0x4000
#define MC_TO_BP_REQ_FIFO_ADDR 0x5000
#define MC_TO_BP_REQ_ENTRIES_ADDR 0x6000

uint64_t bp_get_hart();

void bp_barrier_end(volatile uint64_t *barrier_address, uint64_t total_num_cores);

void bp_hprint(uint8_t hex);

void bp_cprint(uint8_t ch);

void bp_finish(uint8_t code);

#define BP_CFG_BASE_ADDR ((char *)(0x00200000))

int bp_hb_get_credits();

void bp_hb_write_to_manycore_bridge(hb_mc_packet_t *pkt);

int bp_hb_read_from_manycore_bridge(hb_mc_packet_t *pkt, hb_mc_fifo_rx_t type);

#ifdef __cplusplus
}
#endif

#endif
