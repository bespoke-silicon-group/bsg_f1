#define DEBUG
#include <bsg_manycore.h>
#include <bsg_manycore_eva.h>
#include <bsg_manycore_printing.h>
#include <bsg_manycore_tile.h>
#include <inttypes.h>
#include "../cl_manycore_regression.h"
#include "test_manycore_parameters.h"
#define TEST_NAME "test_manycore_eva_read_write"
#define DATA_WORDS 16

int check_eva_read(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, hb_mc_coordinate_t *target, hb_mc_eva_t *eva, uint32_t *gold_data, size_t nwords){
	uint32_t read_data[nwords];
	int err, errors, i;

	for (i = 0; i < nwords; ++i) {
		read_data[i] = ~gold_data[i];
	}

	err = hb_mc_manycore_eva_read(mc, map, target, eva,
				read_data, nwords*sizeof(*read_data));
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: failed to read from manycore at eva 0x%x: %s\n",
			   __func__,
			hb_mc_strerror(err),
			eva);
		return HB_MC_FAIL;
	}

	errors = 0;
	for (i = 0; i < nwords; ++i) {
		if (read_data[i] == gold_data[i]) {
			bsg_pr_test_info("Read back data written: 0x%08" PRIx32 "\n",
					read_data[i]);
		} else {
			bsg_pr_err("Data mismatch: read 0x%08" PRIx32 ", wrote 0x%08" PRIx32 "\n",
					read_data[i], gold_data[i]);
			errors +=1;
		}
	}

	if(!errors){
		bsg_pr_test_info("Completed EVA read Validation\n");
		return HB_MC_SUCCESS;
	}

	bsg_pr_err("Read Validation Failed\n");
	return HB_MC_FAIL;
}

int check_npa_read(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, hb_mc_coordinate_t *target, hb_mc_eva_t *eva, uint32_t *gold_data, size_t nwords){
	uint32_t read_data[nwords];
	int err, errors, i;
	size_t sz;
	hb_mc_npa_t npa;
	const hb_mc_config_t *cfg;

	cfg = hb_mc_manycore_get_config(mc);
	err = hb_mc_eva_to_npa(cfg, map, target, eva, &npa, &sz);
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: failed to translate EVA 0x%x to NPA: %s\n",
			__func__,
			eva,
			hb_mc_strerror(err));
		return HB_MC_FAIL;
	}
	bsg_pr_test_info("%s: EVA 0x%x mapped to NPA (x: %d, y: %d, EPA, 0x%x)\n",
		__func__,
		eva,
		hb_mc_npa_get_x(&npa),
		hb_mc_npa_get_y(&npa),
		hb_mc_npa_get_epa(&npa));

	err = hb_mc_manycore_read_mem(mc, &npa, read_data, nwords * sizeof(*read_data));
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: failed to read from manycore NPA (x :%d, y: %d, EPA: 0x%x): %s\n",
			__func__,
			hb_mc_npa_get_x(&npa),
			hb_mc_npa_get_y(&npa),
			hb_mc_npa_get_epa(&npa),
			hb_mc_strerror(err));
		return HB_MC_FAIL;
	}

	errors = 0;
	for (i = 0; i < nwords; ++i) {
		if (read_data[i] == gold_data[i]) {
			bsg_pr_test_info("Read back data written: 0x%08" PRIx32 "\n",
					read_data[i]);
		} else {
			bsg_pr_err("Data mismatch: read 0x%08" PRIx32 ", wrote 0x%08" PRIx32 "\n",
					read_data[i], gold_data[i]);
			errors +=1;
		}
	}

	if(!errors){
		bsg_pr_test_info("Completed NPA read Validation\n");
		return HB_MC_SUCCESS;
	}

	bsg_pr_err("Read Validation Failed\n");
	return HB_MC_FAIL;
}
/* Do a Memset-Read (Verify) - Write - Read (Verify) test */
int test_eva_srwr(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, hb_mc_coordinate_t *target, hb_mc_eva_t *eva_dest, uint8_t memset_char, uint32_t *write_data, size_t nwords)
{
	uint32_t read_data[nwords];
	uint32_t memset_word;
	int err, i;
	uint32_t memset_data[nwords];

	memset_word = (memset_char << 24) | (memset_char << 16) | (memset_char << 8) | memset_char;
	/***************************/
	/* Running Memset          */
	/***************************/
	bsg_pr_test_info("Memset at EVA 0x%08x\n", eva_dest);
	err = hb_mc_manycore_eva_memset(mc, map, target, eva_dest,
					memset_char, nwords*sizeof(*write_data));
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: failed to memset manycore: %s\n",
			   __func__,
			   hb_mc_strerror(err));
		return err;
	}

	for(i = 0; i < nwords; ++i){
		memset_data[i] = memset_word;
	}

	/***************************************/
	/* Read back Data from Memory using EVA*/
	/***************************************/
	err = check_eva_read(mc, map, target, eva_dest, memset_data, nwords);
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: Read validation of manycore memset using EVA failed: %s\n",
			   __func__,
			   hb_mc_strerror(err));
		return err;
	}

	/***************************************/
	/* Read back Data from Memory using NPA*/
	/***************************************/
	err = check_npa_read(mc, map, target, eva_dest, memset_data, nwords);
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: Read validation of manycore memset using NPA failed: %s\n",
			   __func__,
			   hb_mc_strerror(err));
		return err;
	}

	bsg_pr_test_info("Memset successful\n");

	/***************************/
	/* Writing via EVA */
	/***************************/
	bsg_pr_test_info("Writing to EVA 0x%08x\n", eva_dest);
	err = hb_mc_manycore_eva_write(mc, map, target, eva_dest, write_data, nwords*sizeof(*write_data));
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: failed to write to manycore: %s\n",
			   __func__,
			   hb_mc_strerror(err));
		return err;
	}
	/***************************************/
	/* Read back Data from Memory using EVA*/
	/***************************************/
	err = check_eva_read(mc, map, target, eva_dest, write_data, nwords);
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: Read validation of manycore EVA write using EVA failed: %s\n",
			   __func__,
			   hb_mc_strerror(err));
		return err;
	}

	/***************************************/
	/* Read back Data from Memory using NPA*/
	/***************************************/
	err = check_npa_read(mc, map, target, eva_dest, write_data, nwords);
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: Read validation of manycore EVA write using NPA failed: %s\n",
			   __func__,
			   hb_mc_strerror(err));
		return err;
	}

	bsg_pr_test_info("Write successful\n");
	return HB_MC_SUCCESS;
}

int test_manycore_eva_read_write () {
	hb_mc_manycore_t manycore = {0}, *mc = &manycore;
	int err, r = HB_MC_FAIL, i;
	uint32_t write_data[DATA_WORDS], read_data[DATA_WORDS] = {0};
	hb_mc_eva_t eva_dest, eva_src;
	hb_mc_coordinate_t target = { .x = 1, .y = 1 };
	hb_mc_idx_t x, y;
	uint8_t memset_char;
	uint32_t memset_word;
	/********/
	/* INIT */
	/********/
	err = hb_mc_manycore_init(mc, TEST_NAME, 0);
	if (err != HB_MC_SUCCESS) {
		bsg_pr_err("%s: failed to initialize manycore: %s\n",
			   __func__, hb_mc_strerror(err));
		return HB_MC_FAIL;
	}

	for (i = 0; i < DATA_WORDS; ++i) {
		write_data[i] = i + 1;
	}

	memset_char = 0xAA;

	/*********************************************/
	/* Test RW to Local DMEM (DMEM within target)*/
	/*********************************************/
	eva_dest = 0x10;

	r = test_eva_srwr(mc, &default_map, &target, &eva_dest, memset_char, write_data, DATA_WORDS);

	if(r != HB_MC_SUCCESS)
		goto cleanup;

	/*********************************************/
	/* Test RW to Local DMEM (DMEM within target)*/
	/* Test should FAIL: EVA is > DMEM SIZE      */
	/*********************************************/
	eva_dest = eva_dest + hb_mc_tile_get_size_dmem(mc, &target);

	bsg_pr_test_info("Trying to write to invalid Local DMEM address\n");
	bsg_pr_test_info("This test should fail...\n");
	r = test_eva_srwr(mc, &default_map, &target, &eva_dest, memset_char, write_data, DATA_WORDS);

	if(r != HB_MC_FAIL)
		goto cleanup;
	bsg_pr_test_info("We're good!\n");
	r = HB_MC_SUCCESS;

	/*********************************************/
	/* Test RW to Group DMEM (DMEM within Group)*/
	/*********************************************/
	x = 1, y = 1;
	eva_dest = 0x10;
	eva_dest = (GROUP_INDICATOR) | (x << GROUP_X_OFFSET) |
		(y << GROUP_Y_OFFSET) | (eva_dest);

	r = test_eva_srwr(mc, &default_map, &target, &eva_dest, memset_char, write_data, DATA_WORDS);

	if(r != HB_MC_SUCCESS)
		goto cleanup;

	/*********************************************/
	/* Test RW to Group DMEM (DMEM within target)*/
	/* Test should FAIL: EVA is > DMEM SIZE      */
	/*********************************************/
	eva_dest = eva_dest + hb_mc_tile_get_size_dmem(mc, &target);

	bsg_pr_test_info("Trying to write to invalid Group DMEM address\n");
	bsg_pr_test_info("This test should fail...\n");
	r = test_eva_srwr(mc, &default_map, &target, &eva_dest, memset_char, write_data, DATA_WORDS);

	if(r != HB_MC_FAIL)
		goto cleanup;
	bsg_pr_test_info("We're good!\n");
	r = HB_MC_SUCCESS;

	/*********************************************/
	/* Test RW to Group DMEM (DMEM within Group)*/
	/*********************************************/
	x = 2, y = 2;
	eva_dest = 0x10;
	eva_dest = (GLOBAL_INDICATOR) | (x << GLOBAL_X_OFFSET) |
		(y << GLOBAL_Y_OFFSET) | (eva_dest);

	r = test_eva_srwr(mc, &default_map, &target, &eva_dest, memset_char, write_data, DATA_WORDS);

	if(r != HB_MC_SUCCESS)
		goto cleanup;

	/*********************************************/
	/* Test RW to Global DMEM (DMEM within target)*/
	/* Test should FAIL: EVA is > DMEM SIZE      */
	/*********************************************/
	eva_dest = eva_dest + hb_mc_tile_get_size_dmem(mc, &target);

	bsg_pr_test_info("Trying to write to invalid Global DMEM address\n");
	bsg_pr_test_info("This test should fail...\n");
	r = test_eva_srwr(mc, &default_map, &target, &eva_dest, memset_char, write_data, DATA_WORDS);

	if(r != HB_MC_FAIL)
		goto cleanup;
	bsg_pr_test_info("We're good!\n");
	r = HB_MC_SUCCESS;

	/*******/
	/* END */
	/*******/
cleanup:
	hb_mc_manycore_exit(mc);
	return r;
}

#ifdef COSIM
void test_main(uint32_t *exit_code) {
	bsg_pr_test_info(TEST_NAME " Regression Test (COSIMULATION)\n");
	int rc = test_manycore_eva_read_write();
	*exit_code = rc;
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return;
}
#else
int main() {
	bsg_pr_test_info(TEST_NAME " Regression Test (F1)\n");
	int rc = test_manycore_eva_read_write();
	bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
	return rc;
}
#endif
