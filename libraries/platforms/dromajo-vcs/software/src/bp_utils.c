// BlackParrot utilities
// Note: This file is compiled instead of the existing code in the perch
// library in order to allow for easy changes to the finish code in the future

#include <stdint.h>
#include <bp_utils.h>
#include <bsg_manycore.h>
#include <bsg_manycore_printing.h>

uint64_t bp_get_hart() {
    uint64_t core_id;
    __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);
    return core_id;
}

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

void bp_finish(uint8_t code) {
  uint64_t core_id;

  __asm__ volatile("csrr %0, mhartid": "=r"(core_id): :);

  *(FINISH_BASE_ADDR+core_id*8) = code;
}

void bp_hprint(uint8_t hex) {

  *(PUTCHAR_BASE_ADDR) = ('0' + hex);
}

void bp_cprint(uint8_t ch) {

  *(PUTCHAR_BASE_ADDR) = ch;
}

/*
 * Reads the manycore bridge for number of credits remaining in the endpoint
 * @returns number of credits in the manycore bridge enddpoint
 */
int bp_hb_get_credits() {
	uint32_t *bp_to_mc_req_credits_addr = (uint32_t *) (MC_BASE_ADDR + BP_TO_MC_REQ_CREDITS_ADDR);
	int credits = (int) *bp_to_mc_req_credits_addr;
	return credits;
}

/*
 * Writes a 128-bit manycore packet in 32-bit chunks to the manycore bridge FIFO
 * @param[in] pkt --> Pointer to the manycore packet
 */
void bp_hb_write_to_manycore_bridge(hb_mc_packet_t *pkt) {
	int credits;

	// Wait till there is atleast 1 credit to send the write
	do {
		credits = bp_hb_get_credits();
	} while (credits == 0);

	uint32_t *bp_to_mc_req_fifo_addr = (uint32_t *) (MC_BASE_ADDR + BP_TO_MC_REQ_FIFO_ADDR);
	for(int i = 0; i < 4; i++) {
		*bp_to_mc_req_fifo_addr = pkt->words[i];
		bp_to_mc_req_fifo_addr++;
	}
}

/*
 * Reads the manycore bridge FIFOs in 32-bit chunks to form the 128-bit packet
 * @param[in] pkt --> Pointer to the manycore packet
 * @param[in] type --> Type of FIFO to read from
 * @returns HB_MC_SUCCESS on success, HB_MC_FAIL if FIFO type is unknown
 */
int bp_hb_read_from_manycore_bridge(hb_mc_packet_t *pkt, hb_mc_fifo_rx_t type) {
	switch(type) {
		case HB_MC_FIFO_RX_REQ:
		{
			// Check if the entries are full
			uint32_t *mc_to_bp_req_entries_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_REQ_ENTRIES_ADDR);
			while(!(*mc_to_bp_req_entries_addr))
				;

			// Read the value
			uint32_t *mc_to_bp_req_fifo_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_REQ_FIFO_ADDR);
			uint32_t read_status = 0xFFFFFFFF;
			for (int i = 0; i < 4; i++) {
				pkt->words[i] = *mc_to_bp_req_fifo_addr;
				read_status &= pkt->words[i];
				mc_to_bp_req_fifo_addr++;
			}

			// If all packets are 0xFFFFFFFF --> there is something wrong
			if (read_status == 0xFFFFFFFF)
				return HB_MC_FAIL;
		}
		break;
		case HB_MC_FIFO_RX_RSP:
		{
			// Check if the entries are full
			uint32_t *mc_to_bp_resp_entries_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_RESP_ENTRIES_ADDR);
			while(!(*mc_to_bp_resp_entries_addr))
				;

			// Read the value
			uint32_t *mc_to_bp_resp_fifo_addr = (uint32_t *) (MC_BASE_ADDR + MC_TO_BP_RESP_FIFO_ADDR);
			uint32_t read_status = 0xFFFFFFFF;
			for (int i = 0; i < 4; i++) {
				pkt->words[i] = *mc_to_bp_resp_fifo_addr;
				read_status &= pkt->words[i];
				mc_to_bp_resp_fifo_addr++;
			}

			// If all packets are 0xFFFFFFFF --> there is something wrong
			if (read_status == 0xFFFFFFFF)
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

