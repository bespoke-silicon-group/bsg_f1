# Copyright (c) 2021, University of Washington All rights reserved.
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

################################################################################
# environment.mk verifies the build environment and sets the following
# makefile variables:
#
# LIBRAIRES_PATH: The path to the libraries directory
# HARDWARE_PATH: The path to the hardware directory
# EXAMPLES_PATH: The path to the examples directory
# BASEJUMP_STL_DIR: Path to a clone of BaseJump STL
# BSG_MANYCORE_DIR: Path to a clone of BSG Manycore
###############################################################################

CURRENT_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REPLICANT_PATH:=$(shell git rev-parse --show-toplevel)

include $(REPLICANT_PATH)/environment.mk
SPMD_SRC_PATH = $(BSG_MANYCORE_DIR)/software/spmd
CUDALITE_SRC_PATH = $(SPMD_SRC_PATH)/bsg_cuda_lite_runtime
GRAPHIT_PATH = $(CURRENT_PATH)/../graphit-src

GRAPH_PATH := $(GRAPHIT_PATH)/test/graphs/darpa-eval/jhu.mtx
# TEST_NAME is the basename of the executable
TEST_NAME = main
# KERNEL_NAME is the name of the CUDA-Lite Kernel
KERNEL_NAME = pr_nibble
HOST_TARGET := $(TEST_NAME).exec

BASE_VERSIONS += hybrid

ITERATIONS := 0 1 2 3 4 5 6 7 8 9
v-from-basev-and-iter = $1-iteration-$2
basev-from-v          = $(word 1,$(subst -iteration-, ,$1))
iter-from-v           = $(word 2,$(subst -iteration-, ,$1))

VERSIONS := $(foreach i,$(ITERATIONS),$(foreach v,$(BASE_VERSIONS),\
        $(call v-from-basev-and-iter,$v,$i)))

VERSION-DIRS := $(foreach v,$(VERSIONS),kernel/$v)

.PHONY: $(VERSION-DIRS)
$(VERSION-DIRS): 
	cp -r $(call basev-from-v,$@) $@

.PHONY: versions bleach-versions
versions: $(VERSION-DIRS)
bleach-versions: 
	rm -rf $(VERSION-DIRS)

DEFAULT_VERSION := hybrid
KERNEL_DEFAULT 	:= kernel/$(DEFAULT_VERSION)/kernel.cpp

###############################################################################
# Host code compilation flags and flow
###############################################################################

# TEST_SOURCES is a list of source files that need to be compiled
TEST_SOURCES = main.cpp

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CDEFINES += 
CXXDEFINES += 

FLAGS     = -g -Wall -Wno-unused-function -Wno-unused-variable
CFLAGS   += -std=c99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS) 


# compilation.mk defines rules for compilation of C/C++
include $(EXAMPLES_PATH)/compilation.mk

# Specify any header file dependencies
main.o: INCLUDES += -I$(CURRENT_PATH) -I$(GRAPHIT_PATH)/src/runtime_lib/

###############################################################################
# Host code link flags and flow
###############################################################################

LDFLAGS += 

# link.mk defines rules for linking of the final execution binary.
include $(EXAMPLES_PATH)/link.mk

###############################################################################
# Device code compilation flow
###############################################################################

# BSG_MANYCORE_KERNELS is a list of manycore executables that should
# be built before executing.
BSG_MANYCORE_KERNELS = kernel.riscv 

kernel.rvo: RISCV_CXX = $(RISCV_GXX)
kernel.riscv: kernel.rvo

%/kernel.rvo: RISCV_CXX = $(RISCV_GXX)

# Tile Group Dimensions
TILE_GROUP_DIM_X = 16
TILE_GROUP_DIM_Y = 8
RISCV_DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_DEFINES += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)

RISCV_INCLUDES += -I$(CURRENT_PATH)/kernel/include -I$(GRAPHIT_PATH)/src/runtime_lib/infra_hb/device/

include $(EXAMPLES_PATH)/cuda/riscv.mk

%/kernel.riscv: crt.rvo bsg_set_tile_x_y.rvo bsg_tile_config_vars.rvo main.rvo %/kernel.rvo $(RISCV_TARGET_OBJECTS) $(RISCV_LINK_SCRIPT)
	$(RISCV_LD) -T $(RISCV_LINK_SCRIPT) $(RISCV_LDFLAGS) $(filter %.rvo,$^) -o $@

###############################################################################
# Execution flow
#
# C_ARGS: Use this to pass arguments that you want to appear in argv
#         For SPMD tests C arguments are: <Path to RISC-V Binary> <Test Name>
#
# SIM_ARGS: Use this to pass arguments to the simulator
###############################################################################
#C_ARGS ?= $(BSG_MANYCORE_KERNELS) $(KERNEL_NAME) -g $(GRAPH_PATH)
C_ARGS ?= $(KERNEL_NAME) -g $(GRAPH_PATH)

SIM_ARGS ?=

# Include platform-specific execution rules
include $(EXAMPLES_PATH)/execution.mk


$(VERSIONS): %: kernel/%/$(HOST_TARGET).log

ALIASES = vanilla_stats.csv vcache_stats.csv dramsim3epoch.json dramsim3.json dramsim3.tag.json dramsim3.txt 
$(ALIASES): $(HOST_TARGET).log ;
$(HOST_TARGET).log: $(HOST_TARGET) kernel.riscv 
	./$(HOST_TARGET) $(SIM_ARGS) +c_args="kernel.riscv $(DEFAULT_VERSION) $(C_ARGS)" 2>&1 | tee $@  


KERNEL_ALIASES = $(foreach a,$(ALIASES),kernel/%/$a)
.PRECIOUS: $(KERNEL_ALIASES) kernel/%/kernel.riscv
$(KERNEL_ALIASES): kernel/%/$(HOST_TARGET).log ;
kernel/%/$(HOST_TARGET).log: $(HOST_TARGET) kernel/%/kernel.riscv 
	$(eval EXEC_PATH   := $(patsubst %/,%,$(dir $@)))
	$(eval KERNEL_PATH := $(CURRENT_PATH)/$(EXEC_PATH))
	$(eval _VERSION    := $(notdir $(EXEC_PATH)))
	cd $(EXEC_PATH) && \
	$(CURRENT_PATH)/$(HOST_TARGET) $(SIM_ARGS) +c_args="$(KERNEL_PATH)/kernel.riscv $(_VERSION) $(C_ARGS)" \
		2>&1 | tee $(notdir $a)

.PRECIOUS: %.log

all-versions: $(foreach v,$(VERSIONS),kernel/$v/$(HOST_TARGET).log)

###############################################################################
# Regression Flow
###############################################################################

regression: versions all-versions 
	@grep "BSG REGRESSION TEST .*PASSED.*" $< > /dev/null

###############################################################################
# Default rules, help, and clean
###############################################################################
.DEFAULT_GOAL := help
help:
	@echo "Usage:"
	@echo "make {clean | $(TEST_NAME).{profile,debug} | $(TEST_NAME).{profile,debug}.log}"
	@echo "      $(TEST_NAME).profile: Build executable with profilers enabled"
	@echo "      $(TEST_NAME).debug: Build waveform executable (if VCS)"
	@echo "      $(TEST_NAME).{profile,debug}.log: Run specific executable"
	@echo "      clean: Remove all subdirectory-specific outputs"

print-%  : ; @echo $* = $($*)

version.clean:
	rm -rf kernel/*/*{.ucli,.csv,.log,.rvo,.riscv,.vpd,.key,.dis,.ll,.ll.s}

.PHONY: clean

clean: bleach-versions version.clean


