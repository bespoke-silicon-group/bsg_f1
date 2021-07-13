// BlackParrot platform-specific header file
// This file contains manycore-specific platform functions and constants

#ifndef BP_HB_PLATFORM_H
#define BP_HB_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bsg_manycore.h>
#include <bsg_manycore_epa.h>

// Host X and Y coordinates
#ifndef HOST_X_COORD
#define HOST_X_COORD 15
#endif

#ifndef HOST_Y_COORD
#define HOST_Y_COORD 8
#endif

// BlackParrot coordinates
// Note: Currently not all of these coordinates are in use
// but might be useful during BlackParrot integration
#ifndef BP_COORDS
#define BP_POD_X 0
#define BP_POD_Y 1
// Coordinates of BP-HB bridge used for manycore communication
#define BP_HOST_LINK_X 15
#define BP_HOST_LINK_Y 9
// Coordinates of BP-HB bridge used for DRAM loads and stores
#define BP_DRAM_0_LINK_X 15
#define BP_DRAM_0_LINK_Y 10
#define BP_DRAM_1_LINK_X 15
#define BP_DRAM_1_LINK_Y 11
#endif

// Memory mapped addresses to interact with the BlackParrot host (aka simulator)
#define HB_MC_HOST_EPA_ARGS_START 0x0000
#define HB_MC_HOST_EPA_ARGS_FINISH 0x00FF
#define HB_MC_HOST_EPA_CONFIG_START 0x0100
#define HB_MC_HOST_EPA_CONFIG_FINISH 0x01FF
#define HB_MC_HOST_EPA_RESET_DONE 0x0200
#define HB_MC_HOST_EPA_TX_VACANT 0x0300

#define HB_MC_HOST_OP_FINISH_CODE 0xFFFFFFFF
#define DROMAJO_RW_FAIL_CODE 0xFFFFFFFF

// Memory mapped addresses to interact with the manycore bridge
// BlackParrot currently sets the MSB of its address to 1 to commnicate with the manycore
// Dromajo however is setup to identify the manycore as a device
#define MC_BASE_ADDR 0x500000
#define BP_TO_MC_REQ_FIFO_ADDR 0x1000
#define BP_TO_MC_REQ_CREDITS_ADDR 0x2000
#define MC_TO_BP_RESP_FIFO_ADDR 0x3000
#define MC_TO_BP_RESP_ENTRIES_ADDR 0x4000
#define MC_TO_BP_REQ_FIFO_ADDR 0x5000
#define MC_TO_BP_REQ_ENTRIES_ADDR 0x6000

/**************************** Manycore Bridge helper functions ****************************/
// Use these functions to enable Dromajo/BlackParrot to write to the bridge. These functions
// are undefined and carry no meaning when used by the BlackParrot host (aka simulator)

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

#ifdef __cplusplus
}
#endif

#endif