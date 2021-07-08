// BlackParrot utilities
// Compile this file instead of the existing one in the BlackParrot firmware

#include <stdint.h>
#include <bp_utils.h>
#include <bsg_manycore.h>
#include <bsg_manycore_printing.h>

/*
 * Reads the manycore bridge for number of credits used in the endpoint
 * @param[in] credits_used --> Pointer to a location in memory that will hold the number of credits used
 * @returns HB_MC_SUCCESS
 */
int bp_hb_get_credits_used(int *credits_used) {
	uint32_t *bp_to_mc_req_credits_addr = (uint32_t *) (MC_BASE_ADDR + BP_TO_MC_REQ_CREDITS_ADDR);
	*credits_used = (int) *bp_to_mc_req_credits_addr;
	if (*credits_used < 0) {
		bsg_pr_err("Credits used cannot be negative. Credits used = %d", *credits_used);
		return HB_MC_FAIL;
	}
	return HB_MC_SUCCESS;
}

/*
 * Writes a 128-bit manycore packet in 32-bit chunks to the manycore bridge FIFO
 * @param[in] pkt --> Pointer to the manycore packet
 * @returns HB_MC_SUCCESS
 * TODO: Implement error checking (Requires some modifications in Dromajo)
 */
int bp_hb_write_to_mc_bridge(hb_mc_packet_t *pkt) {
	uint32_t *bp_to_mc_req_fifo_addr = (uint32_t *) (MC_BASE_ADDR + BP_TO_MC_REQ_FIFO_ADDR);
	for(int i = 0; i < 4; i++) {
		*bp_to_mc_req_fifo_addr = pkt->words[i];
		bp_to_mc_req_fifo_addr++;
	}
	return HB_MC_SUCCESS;
}

/*
 * Checks if the MC to BP FIFO contains any valid elements to be read
 * @param[in] entries --> Pointer to a location in memory that will hold the number of entries
 * @param[in] type --> Type of FIFO to read from
 * @returns HB_MC_SUCCESS on success or HB_MC_FAIL on fail
 */
int bp_hb_get_fifo_entries(int *entries, hb_mc_fifo_rx_t type) {
	switch (type) {
		case HB_MC_FIFO_RX_REQ:
		{
			uint32_t *mc_to_bp_req_fifo_entries_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_REQ_ENTRIES_ADDR);
			*entries = *mc_to_bp_req_fifo_entries_addr;
			if (*entries < 0) {
				bsg_pr_err("Entries occupied cannot be negative. Entries = %d", *entries);
				return HB_MC_FAIL;
			}
		}
		break;
		case HB_MC_FIFO_RX_RSP:
		{
			uint32_t *mc_to_bp_resp_fifo_entries_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_RESP_ENTRIES_ADDR);
			*entries = *mc_to_bp_resp_fifo_entries_addr;
			if (*entries < 0) {
				bsg_pr_err("Entries occupied cannot be negative. Entries = %d", *entries);
				return HB_MC_FAIL;
			}
		}
		break;
		default:
		{
			bsg_pr_err("%s: Unknown packet type\n", __func__);
      return HB_MC_FAIL;
		}
		break;
	}
	return HB_MC_SUCCESS;
}

/*
 * Reads the manycore bridge FIFOs in 32-bit chunks to form the 128-bit packet
 * @param[in] pkt --> Pointer to the manycore packet
 * @param[in] type --> Type of FIFO to read from
 * @returns HB_MC_SUCCESS on success, HB_MC_FAIL if FIFO type is unknown
 */
int bp_hb_read_from_mc_bridge(hb_mc_packet_t *pkt, hb_mc_fifo_rx_t type) {
	switch(type) {
		case HB_MC_FIFO_RX_REQ:
		{
			uint32_t *mc_to_bp_req_fifo_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_REQ_FIFO_ADDR);
			uint32_t fifo_read_status = DROMAJO_RW_FAIL_CODE;
			do {
				for (int i = 0; i < 4; i++) {
					pkt->words[i] = *mc_to_bp_req_fifo_addr;
					fifo_read_status &= pkt->words[i];
					mc_to_bp_req_fifo_addr++;
				}
			} while (fifo_read_status == DROMAJO_RW_FAIL_CODE);

			// There is something wrong if the read status is equal to the FAIL code
			if (fifo_read_status == DROMAJO_RW_FAIL_CODE)
				return HB_MC_FAIL;
		}
		break;
		case HB_MC_FIFO_RX_RSP:
		{
			uint32_t *mc_to_bp_resp_fifo_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_RESP_FIFO_ADDR);
			uint32_t fifo_read_status = DROMAJO_RW_FAIL_CODE;
			do {
				for (int i = 0; i < 4; i++) {
					pkt->words[i] = *mc_to_bp_resp_fifo_addr;
					fifo_read_status &= pkt->words[i];
					mc_to_bp_resp_fifo_addr++;
				}
			} while(fifo_read_status == DROMAJO_RW_FAIL_CODE);

			// There is something wrong if the read status is equal to the FAIL code
			if (fifo_read_status == DROMAJO_RW_FAIL_CODE)
				return HB_MC_FAIL;
		}
		break;
		default:
		{
			bsg_pr_err("%s: Unknown packet type\n", __func__);
      return HB_MC_FAIL;
		}
		break;
	}
	return HB_MC_SUCCESS;
}

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
	finish_pkt.request.x_dst = (0 << 4) | 0;
	finish_pkt.request.y_dst = (1 << 3) | 0;
	finish_pkt.request.x_src = (0 << 4) | 0;
	finish_pkt.request.y_src = (1 << 3) | 1;
	finish_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_SW;
  finish_pkt.request.payload = (core_id << 16) | (0x0000FFFF & code);
  finish_pkt.request.reg_id = 0;
	if (code >= 0)
		finish_pkt.request.addr = MC_FINISH_EPA_ADDR;
	else
		finish_pkt.request.addr = MC_FAIL_EPA_ADDR;

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
	hprint_pkt.request.x_dst = (0 << 4) | 0;
	hprint_pkt.request.y_dst = (1 << 3) | 0;
	hprint_pkt.request.x_src = (0 << 4) | 0;
	hprint_pkt.request.y_src = (1 << 3) | 1;
	hprint_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_SW;
  hprint_pkt.request.payload = ('0' + hex);
  hprint_pkt.request.reg_id = 0;
	hprint_pkt.request.addr = MC_STDOUT_EPA_ADDR;

	int	err = bp_hb_write_to_mc_bridge(&hprint_pkt);
}

/*
 * Sends a character to the host to print
 * @param[in] ch --> Character to print
 */
void bp_cprint(uint8_t ch) {

  hb_mc_packet_t cprint_pkt;
	cprint_pkt.request.x_dst = (0 << 4) | 0;
	cprint_pkt.request.y_dst = (1 << 3) | 0;
	cprint_pkt.request.x_src = (0 << 4) | 0;
	cprint_pkt.request.y_src = (1 << 3) | 1;
	cprint_pkt.request.op_v2 = HB_MC_PACKET_OP_REMOTE_SW;
  cprint_pkt.request.payload = ch;
  cprint_pkt.request.reg_id = 0;
	cprint_pkt.request.addr = MC_STDOUT_EPA_ADDR;

	int err = bp_hb_write_to_mc_bridge(&cprint_pkt);
}