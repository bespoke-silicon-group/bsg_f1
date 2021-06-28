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

# This Makefile fragment is for building the dramsim3 library for
# cosimulation
ORANGE=\033[0;33m
RED=\033[0;31m
NC=\033[0m

# This file REQUIRES several variables to be set. They are typically
# set by the Makefile that includes this makefile..
#

# CL_DIR: The path to the root of the BSG F1 Repository
ifndef CL_DIR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: CL_DIR is not defined$(NC)"))
endif

# LIBRARIES_PATH: The path to the regression folder in BSG F1
ifndef LIBRARIES_PATH
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: LIBRARIES_PATH is not defined$(NC)"))
endif

# Don't include more than once
ifndef (_BSG_F1_TESTBENCHES_DRAMSIM3_MK)
_BSG_F1_TESTBENCHES_DRAMSIM3_MK := 1
# Check if dramsim3 is the memory model for this design
ifneq ($(filter hbm2, $(subst _, ,$(BSG_MACHINE_MEM_CFG))),)

# Add a clean rule
.PHONY: dramsim3.clean

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: LDFLAGS += -L$(LIBRARIES_PATH)/features/dma/simulation -Wl,-rpath=$(LIBRARIES_PATH)/features/dma/simulation -ldramsim3
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: $(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so

# Rules for building dramsim3 library
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS := -std=c++11 -D_GNU_SOURCE -D_DEFAULT_SOURCE -Wall -fPIC -shared
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS += -I$(BASEJUMP_STL_DIR)/imports/DRAMSim3/src
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS += -I$(BASEJUMP_STL_DIR)/imports/DRAMSim3/ext/headers
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS += -I$(BASEJUMP_STL_DIR)/imports/DRAMSim3/ext/fmt/include
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS += -DFMT_HEADER_ONLY=1
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS += -DBASEJUMP_STL_DIR="$(BASEJUMP_STL_DIR)"
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXXFLAGS += -DBLOOD_GRAPH=1
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: CXX=g++

$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/bankstate.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/channel_state.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/command_queue.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/common.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/configuration.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/controller.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/dram_system.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/hmc.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/memory_system.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/refresh.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/simple_stats.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/timing.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/imports/DRAMSim3/src/blood_graph.cc
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: $(BASEJUMP_STL_DIR)/bsg_test/bsg_dramsim3.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -Wl,-soname,$(notdir $@) -o $@

endif # ifneq ($(filter dramsim3, $(subst _, ,$(BSG_MACHINE_MEM_CFG))),)
dramsim3.clean:
	rm -f $(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so

# Add as a subrule to simlibs.clean
libraries.clean: dramsim3.clean
endif # ifndef(_BSG_F1_TESTBENCHES_DRAMSIM3_MK)

