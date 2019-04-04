#include <stdlib.h>
#ifndef COSIM
	#include <bsg_manycore_mem.h>
	#include <bsg_manycore_loader.h>
	#include <bsg_manycore_driver.h>
	#include <bsg_manycore_errno.h>
#else
	#include "bsg_manycore_mem.h"
	#include "bsg_manycore_loader.h"
	#include "bsg_manycore_driver.h"
	#include "bsg_manycore_errno.h"
#endif

/*! 
 * Copies data from Manycore to host.
 * @param x destination x coordinate
 * @param y destination y coordinate
 * @param epa tile's physical address
 * @param size number of words to copy
 * @return whether or not transaction was successful
 * */
int hb_mc_copy_from_epa (uint8_t fd, request_packet_t *buf, uint32_t x, uint32_t y, uint32_t epa, uint32_t size) {
	if (hb_mc_check_device(fd) != HB_MC_SUCCESS) {
		printf("hb_mc_copy_from_epa(): device was not initialized.\n");
		return HB_MC_FAIL;
	}

//	if (!can_read(size)) {
//		printf("hb_xeon_to_copy(): not enough space to read.\n");
//		return false;
//	}

	packet_t requests[size]; 	
	uint32_t base_byte = epa << 2;
	for (int i = 0; i < size; i++) {
		uint32_t addr = (base_byte + i * sizeof(uint32_t)) >> 2;
		uint32_t data = 0; /* unused */
		hb_mc_format_request_packet(&requests[i].request, addr, data, x, y, OP_REMOTE_LOAD);
	} 
	
	int pass_requests = HB_MC_SUCCESS; /* whether or not load requests send properly */
	for (int i = 0; i < size; i++) {
		if (hb_mc_write_fifo(fd, 0, &requests[i]) != HB_MC_SUCCESS) {
			pass_requests = HB_MC_FAIL;
			break;
		}
	}

	if (pass_requests != HB_MC_SUCCESS) {
		printf("hb_mc_copy_from_epa(): error when sending load request to Manycore.\n");
	}
	
	/* read receive packets from Manycore. TODO: can result in infinite loop. */
	for (int i = 0; i < size; i++) {
		hb_mc_read_fifo(fd, 0, &buf[i]);
	}

	return pass_requests;
}

/*! 
 * Copies data from host to manycore
 * @param x destination x coordinate
 * @param y destination y coordinate
 * @param epa tile's physical address
 * @param size number of words to copy
 * @return whether or not transaction was successful
 * */
int hb_mc_copy_to_epa (uint8_t fd, uint32_t x, uint32_t y, uint32_t epa, uint32_t *buf, uint32_t size) {
	if (hb_mc_check_device(fd) != HB_MC_SUCCESS) {
		printf("hb_xeon_to_epa_copy(): device was not initialized.\n");
		return HB_MC_FAIL;
	}
	packet_t packets[size];
	uint32_t base_byte = epa << 2;
	for (int i = 0; i < size; i++) {
		uint32_t addr = (base_byte + i * sizeof(uint32_t)) >> 2;
		uint32_t data = buf[i];
		hb_mc_format_request_packet(&packets[i].request, addr, data, x, y, OP_REMOTE_STORE);
	} 
	int pass = HB_MC_SUCCESS;
	for (int i = 0; i < size; i++) {
		//printf("hb_mc_copy_to_epa(): ");	
		if (hb_mc_write_fifo(fd, 0, &packets[i]) != HB_MC_SUCCESS) {
			pass = HB_MC_FAIL;
			break;
		}
	}
	if (pass != HB_MC_SUCCESS)
		printf("hb_copy_to_epa(): error when writing to Manycore.\n");

	return pass;
}

