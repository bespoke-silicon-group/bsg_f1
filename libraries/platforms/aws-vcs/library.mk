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

ifndef VCS_HOME
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: VCS_HOME environment variable undefined. Are you sure vcs-mx is installed?$(NC)"))
endif

# aws-fpga and aws-vcs are identical, EXCEPT for the MMIO
# layer. Therefore, we reuse the bsg_manycore_platform.cpp file in
# aws-fpga, but provide our own bsg_manycore_mmio.cpp file that
# handles DPI-based MMIO.
PLATFORM_CXXSOURCES += $(BSG_F1_DIR)/libraries/platforms/aws-vcs/bsg_manycore_mmio.cpp
PLATFORM_CXXSOURCES += $(BSG_F1_DIR)/libraries/platforms/aws-fpga/bsg_manycore_platform.cpp

PLATFORM_OBJECTS += $(patsubst %cpp,%o,$(PLATFORM_CXXSOURCES))
PLATFORM_OBJECTS += $(patsubst %c,%o,$(PLATFORM_CSOURCES))

# -DCOSIM is still necessary, for now
$(LIB_OBJECTS): CXXFLAGS += -DCOSIM
$(LIB_OBJECTS): CFLAGS += -DCOSIM
$(PLATFORM_OBJECTS): INCLUDES := -I$(LIBRARIES_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_F1_DIR)/libraries/platforms/aws-fpga
$(PLATFORM_OBJECTS): INCLUDES += -I$(VCS_HOME)/linux64/lib/
$(PLATFORM_OBJECTS): INCLUDES += -I$(SDK_DIR)/userspace/include
$(PLATFORM_OBJECTS): INCLUDES += -I$(HDK_DIR)/common/software/include

# C/C++ memory system libraries. These will add dependencies to
# $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0.
include $(TESTBENCH_PATH)/dramsim3.mk
include $(TESTBENCH_PATH)/infmem.mk
include $(TESTBENCH_PATH)/libdmamem.mk

# libfpga_mgmt is the platform library provided by AWS. It mirrors the
# same library on AWS F1 hardware.
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: LDFLAGS +=-L$(BSG_PLATFORM_PATH) -Wl,-rpath=$(BSG_PLATFORM_PATH) -lfpga_mgmt
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: $(BSG_PLATFORM_PATH)/libfpga_mgmt.so

$(BSG_PLATFORM_PATH)/libfpga_mgmt.so: INCLUDES := -I$(SDK_DIR)/userspace/include
$(BSG_PLATFORM_PATH)/libfpga_mgmt.so: INCLUDES += -I$(HDK_DIR)/common/software/include
$(BSG_PLATFORM_PATH)/libfpga_mgmt.so: CFLAGS = -std=c11 -D_GNU_SOURCE -fPIC -shared
$(BSG_PLATFORM_PATH)/libfpga_mgmt.so: % : $(SDK_DIR)/userspace/utils/sh_dpi_tasks.c
$(BSG_PLATFORM_PATH)/libfpga_mgmt.so: % : $(HDK_DIR)/common/software/src/fpga_pci_sv.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -Wl,-soname,$(notdir $@) -o $@

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1: %: %.0
	ln -sf $@.0 $@

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so: %: %.1
	ln -sf $@.1 $@

.PHONY: platform.clean
platform.clean:
	rm -f $(PLATFORM_OBJECTS)
	rm -f $(BSG_PLATFORM_PATH)/libfpga_mgmt.so
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1

libraries.clean: platform.clean
