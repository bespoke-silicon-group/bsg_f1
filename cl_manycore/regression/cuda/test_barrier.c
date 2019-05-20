#include "test_barrier.h"

/*!
 * Runs a barrier kernel on a 2x2 tile group. 
 * This tests uses the software/spmd/bsg_cuda_lite_runtime/barrier/ Manycore binary in the dev_cuda_v4 branch of the BSG Manycore github repository.  
*/

int kernel_barrier () {
	fprintf(stderr, "Running the CUDA Barrier Kernel on a 2x2 tile group.\n\n");


	/*****************************************************************************************************************
	* Define the dimension of tile pool.
	* Define path to binary.
	* Initialize device, load binary and unfreeze tiles.
	******************************************************************************************************************/
	device_t device;
	uint8_t mesh_dim_x = 4;
	uint8_t mesh_dim_y = 4;
	uint8_t mesh_origin_x = 0;
	uint8_t mesh_origin_y = 1;
	eva_id_t eva_id = 0;
	char* elf = BSG_STRINGIFY(BSG_MANYCORE_DIR) "/software/spmd/bsg_cuda_lite_runtime" "/barrier/main.riscv";

	hb_mc_device_init(&device, eva_id, elf, mesh_dim_x, mesh_dim_y, mesh_origin_x, mesh_origin_y);


	/*****************************************************************************************************************
	* Define grid_dim_x/y: total number of tile groups
	* Define tg_dim_x/y: number of tiles in each tile group
	* Calculate grid_dim_x/y: number of tile groups needed based on block_size_x/y
	******************************************************************************************************************/
	uint8_t grid_dim_x = 1;
	uint8_t grid_dim_y = 1;
	uint8_t tg_dim_x = 2;
	uint8_t tg_dim_y = 2;


	/*****************************************************************************************************************
	* Prepare list of input arguments for kernel.
	******************************************************************************************************************/
	int argv[1];


	/*****************************************************************************************************************
	* Enquque grid of tile groups, pass in grid and tile group dimensions, kernel name, number and list of input arguments
	******************************************************************************************************************/
	hb_mc_grid_init (&device, grid_dim_x, grid_dim_y, tg_dim_x, tg_dim_y, "kernel_barrier", 0, argv);
	

	/*****************************************************************************************************************
	* Launch and execute all tile groups on device and wait for all to finish. 
	******************************************************************************************************************/
	hb_mc_device_tile_groups_execute(&device);
	

	/*****************************************************************************************************************
	* Freeze the tiles and memory manager cleanup. 
	******************************************************************************************************************/
	hb_mc_device_finish(&device); /* freeze the tiles and memory manager cleanup */

	return HB_MC_SUCCESS;
}

#ifdef COSIM
void test_main(uint32_t *exit_code) {	
	bsg_pr_test_info("test_barrier Regression Test (COSIMULATION)\n");
	int rc = kernel_barrier();
	*exit_code = rc;
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return;
}
#else
int main() {
	bsg_pr_test_info("test_barrier Regression Test (F1)\n");
	int rc = kernel_barrier();
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return rc;
}
#endif

