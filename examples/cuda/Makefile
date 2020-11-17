# Copyright (c) 2019, University of Washington All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# Redistributions of source code must retain the above copyright notice, this list
# of conditions and the following disclaimer.
# 
# Redistributions in binary form must reproduce the above copyright notice, this
# list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
# 
# Neither the name of the copyright holder nor the names of its contributors may
# be used to endorse or promote products derived from this software without
# specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This Makefile compiles, links, and executes examples Run `make help`
# to see the available targets for the selected platform.

# environment.mk verifies the build environment and sets the following
# makefile variables:
#
# LIBRAIRES_PATH: The path to the libraries directory
# HARDWARE_PATH: The path to the hardware directory
# EXAMPLES_PATH: The path to the examples directory
# BASEJUMP_STL_DIR: Path to a clone of BaseJump STL
# BSG_MANYCORE_DIR: Path to a clone of BSG Manycore
# CL_DIR: Path to the directory of this AWS F1 Project
include ../../environment.mk

REGRESSION_TESTS_TYPE = cuda

###############################################################################
# Examples List
#
# UNIFIED_TESTS use a generic top-level, test_unified_main.c. They depend on a
# self-checking Manycore kernel to send a PASS/FAIL packet to the host
#
# INDEPENDENT_TESTS use a per-test top level, <test_name>.c/cpp. They check
# correctness on the host.
# 
# REGRESSION_TESTS merges the two lists
###############################################################################
UNIFIED_TESTS = test_scalar_print
UNIFIED_TESTS += test_empty
UNIFIED_TESTS += test_tile_info
UNIFIED_TESTS += test_barrier_deprecated
UNIFIED_TESTS += test_barrier
UNIFIED_TESTS += test_striped

INDEPENDENT_TESTS += test_binary_load_buffer
INDEPENDENT_TESTS += test_empty_parallel
INDEPENDENT_TESTS += test_multiple_binary_load
INDEPENDENT_TESTS += test_host_memset
INDEPENDENT_TESTS += test_stack_load
INDEPENDENT_TESTS += test_memory_leak
INDEPENDENT_TESTS += test_dram_load_store
INDEPENDENT_TESTS += test_dram_host_allocated
INDEPENDENT_TESTS += test_dram_device_allocated
INDEPENDENT_TESTS += test_device_memset
INDEPENDENT_TESTS += test_device_memcpy
INDEPENDENT_TESTS += test_vec_add
INDEPENDENT_TESTS += test_vec_add_dma
INDEPENDENT_TESTS += test_vec_add_parallel
INDEPENDENT_TESTS += test_vec_add_parallel_multi_grid
INDEPENDENT_TESTS += test_vec_add_serial_multi_grid
INDEPENDENT_TESTS += test_vec_add_shared_mem
INDEPENDENT_TESTS += test_max_pool2d
INDEPENDENT_TESTS += test_shared_mem
INDEPENDENT_TESTS += test_shared_mem_load_store
INDEPENDENT_TESTS += test_matrix_mul
INDEPENDENT_TESTS += test_matrix_mul_shared_mem

INDEPENDENT_TESTS += test_float_all_ops
INDEPENDENT_TESTS += test_float_vec_add
INDEPENDENT_TESTS += test_float_vec_add_shared_mem
INDEPENDENT_TESTS += test_float_vec_mul
INDEPENDENT_TESTS += test_float_vec_div
INDEPENDENT_TESTS += test_float_vec_exp
INDEPENDENT_TESTS += test_float_vec_sqrt
INDEPENDENT_TESTS += test_float_vec_log
INDEPENDENT_TESTS += test_float_matrix_mul
INDEPENDENT_TESTS += test_float_matrix_mul_shared_mem
INDEPENDENT_TESTS += test_softmax
INDEPENDENT_TESTS += test_log_softmax
INDEPENDENT_TESTS += test_hammer_cache
INDEPENDENT_TESTS += test_profiler
INDEPENDENT_TESTS += test_tracer
INDEPENDENT_TESTS += test_conv1d
INDEPENDENT_TESTS += test_conv2d
INDEPENDENT_TESTS += test_mm_opt

REGRESSION_TESTS = $(UNIFIED_TESTS) $(INDEPENDENT_TESTS)

###############################################################################
# Host code compilation flags and flow
###############################################################################
DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE

CDEFINES   += $(DEFINES)
CXXDEFINES += $(DEFINES)

FLAGS     = -g -Wall
CFLAGS   += -std=c99 $(FLAGS) 
CXXFLAGS += -std=c++11 $(FLAGS)

###############################################################################
# Execution Arguments (C_ARGS)
#
# All CUDA tests require two arguments: The path to the RISC-V Binary loaded
# onto the manycore (KERNEL_PATH), and the test name (TEST_NAME).
#
###############################################################################
SPMD_SRC_PATH = $(BSG_MANYCORE_DIR)/software/spmd
CUDALITE_SRC_PATH = $(SPMD_SRC_PATH)/bsg_cuda_lite_runtime
KERNEL_PATH=$(CUDALITE_SRC_PATH)/$(subst test_,,$(TEST_NAME))/main.riscv
C_ARGS = $(KERNEL_PATH) $(TEST_NAME)

# flow.mk defines all of the host compilationk, link, and execution rules
include $(EXAMPLES_PATH)/flow.mk

###############################################################################
# Kernel Binary Build Rules
###############################################################################

# The default Tile Group size is 2x2
TILE_GROUP_DIM_X = 2
TILE_GROUP_DIM_Y = 2

# Custom Tile-Group Dimensions for mm_opt. Custom tile group
# dimensions for other kernels can be set in similar ways.
$(CUDALITE_SRC_PATH)/mm_opt/main.riscv: TILE_GROUP_DIM_X:=$(CL_MANYCORE_DIM_X)
$(CUDALITE_SRC_PATH)/mm_opt/main.riscv: TILE_GROUP_DIM_Y:=$(CL_MANYCORE_DIM_Y)

# Force rebuild targets that depend on .FORCE, like the
# Manycore/SPMD/CUDA Binaries
.FORCE:

.PHONY: test_%.clean test_%.rule

$(USER_RULES): test_%.rule: $(CUDALITE_SRC_PATH)/%/main.riscv

$(USER_CLEAN_RULES):
	CL_DIR=$(CL_DIR) \
	BSG_MANYCORE_DIR=$(BSG_MANYCORE_DIR) \
	BASEJUMP_STL_DIR=$(BASEJUMP_STL_DIR) \
	BSG_IP_CORES_DIR=$(BASEJUMP_STL_DIR) \
	IGNORE_CADENV=1 \
	BSG_MACHINE_PATH=$(BSG_MACHINE_PATH) \
	$(MAKE) -j1 -C $(CUDALITE_SRC_PATH)/$(subst .clean,,$(subst test_,,$@)) clean

$(CUDALITE_SRC_PATH)/%/main.riscv: $(BSG_MACHINE_PATH)/Makefile.machine.include .FORCE
	CL_DIR=$(CL_DIR) \
	BSG_MANYCORE_DIR=$(BSG_MANYCORE_DIR) \
	BASEJUMP_STL_DIR=$(BASEJUMP_STL_DIR) \
	BSG_IP_CORES_DIR=$(BASEJUMP_STL_DIR) \
	IGNORE_CADENV=1 \
	BSG_MACHINE_PATH=$(BSG_MACHINE_PATH) \
	bsg_tiles_X=$(TILE_GROUP_DIM_X) \
	bsg_tiles_Y=$(TILE_GROUP_DIM_Y) \
	$(MAKE) -j1 -C $(dir $@) clean $(notdir $@)

.PHONY: clean

clean:
	rm -rf $(INDEPENDENT_TESTS) test_loader
