/******************************************************************************/
/* Runs the scalar printing kernel on a 2x2 tile group.                       */
/* Each tile prints specific values using bsg_print_int, bsg_print_unsined,   */
/* bsg_print_hexadecimal, bsg_print_float, bsg_print_float_scientific.        */
/* Grid dimensions are prefixed at 1x1.                                       */
/* This tests uses the software/spmd/bsg_cuda_lite_runtime/scalar_print/      */
/* manycore binary in the BSG Manycore repository.                            */
/******************************************************************************/


#include "test_scalar_print.h"

#define TEST_NAME "test_scalar_print"
#define ALLOC_NAME "default_allocator"


int kernel_scalar_print () {
	bsg_pr_test_info("Running the CUDA Scalar Print Kernel "
                         "on a grid of 2x2 tile groups.\n\n");
	int rc;

	srand(time); 



        /**********************************************************************/
        /* Define path to binary.                                             */
        /* Initialize device, load binary and unfreeze tiles.                 */
        /**********************************************************************/
	hb_mc_device_t device;
	rc = hb_mc_device_init(&device, TEST_NAME, 0);
	if (rc != HB_MC_SUCCESS) { 
		bsg_pr_err("failed to initialize device.\n");
		return rc;
	}

	char* elf = BSG_STRINGIFY(BSG_MANYCORE_DIR) "/software/spmd/bsg_cuda_lite_runtime"
                                                    "/scalar_print/main.riscv";
	rc = hb_mc_device_program_init(&device, elf, ALLOC_NAME, 0);
	if (rc != HB_MC_SUCCESS) { 
		bsg_pr_err("failed to initialize program.\n");
		return rc;
	}


        /**********************************************************************/
	/* Define block_size_x/y: amount of work for each tile group          */
	/* Define tg_dim_x/y: number of tiles in each tile group              */
	/* Calculate grid_dim_x/y: number of                                  */
        /* tile groups needed based on block_size_x/y                         */
        /**********************************************************************/
	hb_mc_dimension_t tg_dim = { .x = 2, .y = 2 }; 

	hb_mc_dimension_t grid_dim = { .x = 1, .y = 1 };


        /**********************************************************************/
	/* Prepare list of input arguments for kernel.                        */
        /**********************************************************************/
	int argv[1] = {};


        /**********************************************************************/
	/* Enquque grid of tile groups, pass in grid and tile group dimensions*/
        /* kernel name, number and list of input arguments                    */
        /**********************************************************************/
	rc = hb_mc_application_init (&device, grid_dim, tg_dim, "kernel_scalar_print", 0, argv);
	if (rc != HB_MC_SUCCESS) { 
		bsg_pr_err("failed to initialize grid.\n");
		return rc;
	}


        /**********************************************************************/
	/* Launch and execute all tile groups on device and wait for finish.  */ 
        /**********************************************************************/
	rc = hb_mc_device_tile_groups_execute(&device);
	if (rc != HB_MC_SUCCESS) { 
		bsg_pr_err("failed to execute tile groups.\n");
		return rc;
	}	


        /**********************************************************************/
        /* Freeze the tiles and memory manager cleanup.                       */
        /**********************************************************************/
	rc = hb_mc_device_finish(&device); 
	if (rc != HB_MC_SUCCESS) { 
		bsg_pr_err("failed to de-initialize device.\n");
		return rc;
	}

	return HB_MC_SUCCESS;
}

#ifdef COSIM
void cosim_main(uint32_t *exit_code, char * args) {
        // We aren't passed command line arguments directly so we parse them
        // from *args. args is a string from VCS - to pass a string of arguments
        // to args, pass c_args to VCS as follows: +c_args="<space separated
        // list of args>"
        int argc = get_argc(args);
        char *argv[argc];
        get_argv(args, argc, argv);

#ifdef VCS
	svScope scope;
	scope = svGetScopeFromName("tb");
	svSetScope(scope);
#endif
	bsg_pr_test_info("test_scalar_print Regression Test (COSIMULATION)\n");
	int rc = kernel_scalar_print();
	*exit_code = rc;
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return;
}
#else
int main(int argc, char ** argv) {
	bsg_pr_test_info("test_scalar_print Regression Test (F1)\n");
	int rc = kernel_scalar_print();
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return rc;
}
#endif

