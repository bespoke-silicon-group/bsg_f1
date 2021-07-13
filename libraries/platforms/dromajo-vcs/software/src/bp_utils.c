// BlackParrot utilities' definitions

#include <stdint.h>
#include <bp_utils.h>

/*
 * Get hardware thread ID
 * @returns result of reading the mhartid register
 */
uint64_t bp_get_hart() {
    uint64_t core_id;
    __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);
    return core_id;
}

/*
 * Checks to see if a barrier is finished
 * @param[in] barrier_address --> An address in memory that all cores to write to after hitting the barrier
 * @param[in] total_num_cores --> Number of cores
 */
void bp_barrier_end(volatile uint64_t * barrier_address, uint64_t total_num_cores) {
    uint64_t core_id;
    uint64_t atomic_inc = 1;
    uint64_t atomic_result;
    __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);
    
    /* if we're not core 0, increment the barrier and then just loop */
    if (core_id != 0) {
        __asm__ volatile("amoadd.d %0, %2, (%1)": "=r"(atomic_result) 
                                                : "r"(barrier_address), "r"(atomic_inc)
                                                :);
        while (1) { }
    }
    /* 
     * if we're core 0, increment the barrier as well and then test if the
     * barrier is equal to the total number of cores
     */
    else {
        uint64_t finish_value = 0;
        __asm__ volatile("amoadd.d %0, %2, (%1)": "=r"(atomic_result) 
                                                : "r"(barrier_address), "r"(atomic_inc)
                                                :);
        while(*barrier_address < total_num_cores) {

            
        }
        bp_finish(0);
    }
}

/*
 * Sends a finish packet to the host
 * @param[in] code --> Finish code
 */
void bp_finish(int16_t code) {
  uint64_t core_id;

	// Fixme: Core IDs for all BP cores might be zero. How do we handle this?
  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);

  // Finish packet contains core id at the top 16 bits and
	// finish code at the bottom 16 bits
	// zero --> success code
	// nonzero --> fail code
	hb_mc_packet_t finish_pkt;
	finish_pkt.request.x_dst = HOST_X_COORD;
	finish_pkt.request.y_dst = HOST_Y_COORD;
	finish_pkt.request.x_src = (0 << 4) | 0;
	finish_pkt.request.y_src = (1 << 3) | 1;
	finish_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_SW;
	finish_pkt.request.payload = (core_id << 16) | (0x0000FFFF & code);
	finish_pkt.request.reg_id = 0;
	if (code >= 0)
		finish_pkt.request.addr = HB_MC_HOST_EPA_FINISH;
	else
		finish_pkt.request.addr = HB_MC_HOST_EPA_FAIL;

	int err;
	do {
		err = bp_hb_write_to_mc_bridge(&finish_pkt);
	} while (err != HB_MC_SUCCESS);
}

/*
 * Sends a hex digit to the host to print
 * @param[in] hex --> Hex digit to print
 */
void bp_hprint(uint8_t hex) {

	hb_mc_packet_t hprint_pkt;
	hprint_pkt.request.x_dst = HOST_X_COORD;
	hprint_pkt.request.y_dst = HOST_Y_COORD;
	hprint_pkt.request.x_src = (0 << 4) | 0;
	hprint_pkt.request.y_src = (1 << 3) | 1;
	hprint_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_SW;
	hprint_pkt.request.payload = ('0' + hex);
	hprint_pkt.request.reg_id = 0;
	hprint_pkt.request.addr = HB_MC_HOST_EPA_STDOUT;

	int	err = bp_hb_write_to_mc_bridge(&hprint_pkt);
}

/*
 * Sends a character to the host to print
 * @param[in] ch --> Character to print
 */
void bp_cprint(uint8_t ch) {

	hb_mc_packet_t cprint_pkt;
	cprint_pkt.request.x_dst = HOST_X_COORD;
	cprint_pkt.request.y_dst = HOST_Y_COORD;
	cprint_pkt.request.x_src = (0 << 4) | 0;
	cprint_pkt.request.y_src = (1 << 3) | 1;
	cprint_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_SW;
	cprint_pkt.request.payload = ch;
	cprint_pkt.request.reg_id = 0;
	cprint_pkt.request.addr = HB_MC_HOST_EPA_STDOUT;

	int err = bp_hb_write_to_mc_bridge(&cprint_pkt);
}
