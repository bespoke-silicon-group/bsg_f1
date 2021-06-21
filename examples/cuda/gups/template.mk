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

REPLICANT_PATH:=$(shell git rev-parse --show-toplevel)

include $(REPLICANT_PATH)/environment.mk

vpath %.cpp $(EXAMPLES_PATH)/cuda/gups
vpath %.c   $(EXAMPLES_PATH)/cuda/gups

# TEST_NAME is the basename of the executable
TEST_NAME = main
# KERNEL_NAME is the name of the CUDA-Lite Kernel
KERNEL_NAME = gups

###############################################################################
# Host code compilation flags and flow
###############################################################################
hammerblade-helpers-dir = $(EXAMPLES_PATH)/cuda/gups/hammerblade-helpers
include $(hammerblade-helpers-dir)/libhammerblade-helpers-host.mk

# TEST_SOURCES is a list of source files that need to be compiled
TEST_SOURCES  = main.cpp

TEST_HEADERS =  $(find $(EXAMPLES_PATH)/cuda/gups/include/ -name *.h)
TEST_HEADERS += $(find $(EXAMPLES_PATH)/cuda/gups/include/ -name *.hpp)

DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_DEFAULT_SOURCE
CDEFINES += 
CXXDEFINES += 

FLAGS     = -O3 -g -Wall -Wno-unused-function -Wno-unused-variable
FLAGS    += -I$(EXAMPLES_PATH)/cuda/gups/include/host
FLAGS    += -I$(EXAMPLES_PATH)/cuda/gups/include/common
CFLAGS   += -std=c99 $(FLAGS)
CXXFLAGS += -std=c++11 $(FLAGS)
CXXFLAGS += $(libhammerblade-helpers-host-interface-cxxflags)

# compilation.mk defines rules for compilation of C/C++
include $(EXAMPLES_PATH)/compilation.mk

###############################################################################
# Host code link flags and flow
###############################################################################

LDFLAGS  += $(libhammerblade-helpers-host-interface-ldflags)

# link.mk defines rules for linking of the final execution binary.
include $(EXAMPLES_PATH)/link.mk

$(TEST_OBJECTS): $(libhammerblade-helpers-host-interface-headers)
$(TEST_OBJECTS): $(libhammerblade-helpers-host-interface-libraries)
$(TEST_OBJECTS): $(TEST_HEADERS)

###############################################################################
# Device code compilation flow
###############################################################################

# BSG_MANYCORE_KERNELS is a list of manycore executables that should
# be built before executing.
BSG_MANYCORE_KERNELS = $(EXAMPLES_PATH)/cuda/gups/main.riscv
#RISCV_TARGET_OBJECTS  = main.riscv.rvo
RISCV_TARGET_OBJECTS += gups-kernel.riscv.rvo
RISCV_INCLUDES += -I$(EXAMPLES_PATH)/cuda/gups/include/kernel
RISCV_INCLUDES += -I$(EXAMPLES_PATH)/cuda/gups/include/common
RISCV_CCPPFLAGS += -D__KERNEL__

TILE_GROUP_DIM_X=2
TILE_GROUP_DIM_Y=2
RISCV_DEFINES += -DTILE_GROUP_DIM_X=$(TILE_GROUP_DIM_X)
RISCV_DEFINES += -DTILE_GROUP_DIM_Y=$(TILE_GROUP_DIM_Y)
RISCV_DEFINES += -Dbsg_tiles_X=$(TILE_GROUP_DIM_X)
RISCV_DEFINES += -Dbsg_tiles_Y=$(TILE_GROUP_DIM_Y)

include $(EXAMPLES_PATH)/cuda/riscv.mk

###############################################################################
# Execution flow
#
# C_ARGS: Use this to pass arguments that you want to appear in argv
#         For SPMD tests C arguments are: <Path to RISC-V Binary> <Test Name>
#
# SIM_ARGS: Use this to pass arguments to the simulator
###############################################################################
C_ARGS ?= $(BSG_MANYCORE_KERNELS) $(KERNEL_NAME) $(EXTRA_C_ARGS)

SIM_ARGS ?=

# Include platform-specific execution rules
include $(EXAMPLES_PATH)/execution.mk

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


.PHONY: clean
