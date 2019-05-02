#include "test_dram_read_write.h"

#define ARRAY_SIZE 1024
#define BASE_ADDR 0x1000

int test_dram_read_write() {
	srand(0);
	int error;
	uint8_t fd; 
	hb_mc_init_host(&fd);	

	if (hb_mc_init_host(&fd) != HB_MC_SUCCESS) {
		fprintf(stderr, "test_dram_read_write(): failed to initialize host.\n");
		return HB_MC_FAIL;
	}
	

	uint32_t manycore_dim_x, manycore_dim_y; 
	error = hb_mc_get_config(fd, HB_MC_CONFIG_DEVICE_DIM_X, &manycore_dim_x); 
	if (error != HB_MC_SUCCESS) { 
		fprintf(stderr, "hb_mc_get_config(): failed to get manycore X dim.\n");
		return HB_MC_FAIL;
	}

	error = hb_mc_get_config(fd, HB_MC_CONFIG_DEVICE_DIM_Y, &manycore_dim_y); 
	if (error != HB_MC_SUCCESS) { 
		fprintf(stderr, "hb_mc_get_config(): failed to get manycore Y dim.\n");
		return HB_MC_FAIL;
	}

 	uint32_t dram_coord_x = 0;
	uint32_t dram_coord_y = manycore_dim_y + 1;




	uint32_t A_host[ARRAY_SIZE];
	uint32_t A_device[ARRAY_SIZE];

	for (int i = 0; i < ARRAY_SIZE; i ++) {
		A_host[i] = rand() % ((1 << 16) - 1); 
	}
	
	for (int i = 0; i < ARRAY_SIZE; i ++) { 
		error = hb_mc_copy_to_epa(fd, dram_coord_x, dram_coord_y, BASE_ADDR + i, &A_host[i], 1); 
		if (error != HB_MC_SUCCESS) {
			fprintf(stderr, "hb_mc_copy_to_epa: failed to write A[%d] = %d to DRAM coord(%d,%d) addr %d.\n", i, A_host[i], dram_coord_x, dram_coord_y, BASE_ADDR + i);
			return HB_MC_FAIL;
		}
	} 



	hb_mc_response_packet_t buf[1]; 
	for (int i = 0; i < ARRAY_SIZE; i ++) {
		error = hb_mc_copy_from_epa(fd, &buf[0], dram_coord_x, dram_coord_y, BASE_ADDR + i, 1); 
		if ( error != HB_MC_SUCCESS) {
			fprintf(stderr, "hb_mc_copy_from_epa: failed to read A[%d] from DRAM coord (%d,%d) addr %d.\n", i, dram_coord_x, dram_coord_y, BASE_ADDR + i);
			return HB_MC_FAIL;
		}
		A_device[i] = hb_mc_response_packet_get_data(&buf[0]);
	}

	int mismatch = 0 ;
	for (int i = 0; i < ARRAY_SIZE; i ++) { 
		if (A_host[i] == A_device[i]) {
			fprintf(stderr, "Success -- A_host[%d] = %d   ==   A_device[%d] = %d.\n", i, A_host[i], i, A_device[i]); 
		}
		else { 
			fprintf(stderr, "Failed -- A_host[%d] = %d   !=   A_device[%d] = %d.\n", i, A_host[i], i, A_device[i]);
			mismatch = 1;
		}
	}

	if (mismatch) 
		return HB_MC_FAIL;
	return HB_MC_SUCCESS;		
}

#ifdef COSIM
void test_main(uint32_t *exit_code) {	
	bsg_pr_test_info("test_dram_read_write Regression Test (COSIMULATION)\n");
	int rc = test_dram_read_write();
	*exit_code = rc;
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return;
}
#else
int main() {
	bsg_pr_test_info("test_dram_read_write Regression Test (F1)\n");
	int rc = test_dram_read_write();
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return rc;
}
#endif
