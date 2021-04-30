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

# Dromajo/BlackParrot uses riscv-newlib for compilation
__NEWLIB = 1
DROMAJO_DIR = $(BLACKPARROT_DIR)/sdk/dromajo

# Compile the platform-level code and features with the library
LIB_CXXSOURCES += $(LIBRARIES_PATH)/features/tracer/noimpl/bsg_manycore_tracer.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/features/profiler/noimpl/bsg_manycore_profiler.cpp
LIB_CXXSOURCES += $(BSG_PLATFORM_PATH)/bsg_manycore_platform.cpp

LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/flockfile.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/funlockfile.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-ba.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-eexst.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-fmtstream.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-fs-xinl.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-help.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-parse.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-pv.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-pvh.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/argp/argp-xinl.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/bp_utils.c
LIB_CSOURCES += $(BSG_PLATFORM_PATH)/software/src/args.c
LIB_CSOURCES += $(BLACKPARROT_DIR)/sdk/perch/bsg_newlib_intf.c
LIB_CSOURCES += $(BLACKPARROT_DIR)/sdk/perch/emulation.c

LIB_SSOURCES += $(BLACKPARROT_DIR)/sdk/perch/atomics.S
LIB_SSOURCES += $(BLACKPARROT_DIR)/sdk/perch/exception.S
LIB_SSOURCES += $(BLACKPARROT_DIR)/sdk/perch/muldiv.S
LIB_SSOURCES += $(BSG_PLATFORM_PATH)/software/src/crt0.S

# For now, use the noimpl version of the features
include $(LIBRARIES_PATH)/features/dma/noimpl/feature.mk

$(DMA_FEATURE_OBJECTS): INCLUDES += -I$(BSG_PLATFORM_PATH)/software/include
$(DMA_FEATURE_OBJECTS): CFLAGS += -march=rv64imafd -mcmodel=medany -mabi=lp64 -D_BSD_SOURCE -D_XOPEN_SOURCE=500
$(DMA_FEATURE_OBJECTS): CXXFLAGS += -march=rv64imafd -mcmodel=medany -mabi=lp64 -D_BSD_SOURCE -D_XOPEN_SOURCE=500

# Add the riscv-newlib specific includes for the library
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES := -I$(BSG_PLATFORM_PATH)/software/include
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSSION): INCLUDES += -I$(BLACKPARROT_DIR)/sdk/perch

# Make the litteFS file system
$(BSG_PLATFORM_PATH)/lfs.o:
	$(MAKE) -C $(BLACKPARROT_DIR)/sdk/bp-tests lfs.cpp
	$(RV_CXX) $(BLACKPARROT_DIR)/sdk/bp-tests/lfs.cpp -c -o $@ $(CXXFLAGS) $(INCLUDES)

# Compile the feature libraries with the manycore runtime
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES += -I$(LIBRARIES_PATH)/features/profiler
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES += -I$(LIBRARIES_PATH)/features/tracer

DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/cutils.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/dromajo_cosim.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/dromajo_main.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/dromajo_manycore.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/dw_apb_uart.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/elf64.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/fs.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/fs_disk.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/iomem.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/json.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/machine.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/pci.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/riscv_cpu.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/riscv_machine.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/softfp.cpp
DROMAJO_CXXSOURCES += $(DROMAJO_DIR)/src/virtio.cpp

DROMAJO_OBJECTS += $(patsubst %cpp,%o,$(DROMAJO_CXXSOURCES))

$(DROMAJO_OBJECTS): INCLUDES := -I$(DROMAJO_DIR)/include
$(DROMAJO_OBJECTS): INCLUDES += -I$(BSG_PLATFORM_PATH)/software/include/dromajo
$(DROMAJO_OBJECTS): CXXFLAGS := -std=c++11 -fPIC -Wall -Wno-parentheses -MMD -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE  -D__STDC_FORMAT_MACROS -DFIFO_MAX_ELEMENTS=$(BSG_MACHINE_IO_EP_CREDITS) -std=gnu++11
$(DROMAJO_OBJECTS): LDFLAGS := -fPIC
$(DROMAJO_OBJECTS): RV_CXX = $(CXX)

PLATFORM_CXXSOURCES += $(BSG_PLATFORM_PATH)/bsg_manycore_simulator.cpp

PLATFORM_OBJECTS += $(patsubst %cpp,%o,$(PLATFORM_CXXSOURCES))
PLATFORM_OBJECTS += $(patsubst %c,%o,$(PLATFORM_CSOURCES))

$(PLATFORM_OBJECTS): INCLUDES := -I$(LIBRARIES_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(EXAMPLES_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(LIBRARIES_PATH)/platforms/dpi-verilator
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_MACHINE_PATH)/notrace/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_PLATFORM_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(VCS_HOME)/linux64/lib/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_MANYCORE_DIR)/testbenches/dpi/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BASEJUMP_STL_DIR)/bsg_test/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BLACKPARROT_DIR)/sdk/dromajo/include
$(PLATFORM_OBJECTS): CFLAGS   := -std=c11 -fPIC -DVCS -D_GNU_SOURCE -DVERILATOR -D_BSD_SOURCE -D_XOPEN_SOURCE=500 $(INCLUDES)
$(PLATFORM_OBJECTS): CXXFLAGS := -std=c++11 -fPIC -DVCS -D_GNU_SOURCE -DVERILATOR -D_BSD_SOURCE -D_XOPEN_SOURCE=500 $(INCLUDES)
$(PLATFORM_OBJECTS): LDFLAGS  := -fPIC
$(PLATFORM_OBJECTS): RV_CXX = $(CXX)

# Mirror the extensions linux installation in /usr/lib provides so
# that we can use -lbsg_manycore_runtime
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a.1: %.a.1: %.so.1.0
	ln -sf $(basename $(basename $@)).so.1.0 $@

$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a.1: %.a.1: %.so.1.0
	ln -sf $(basename $(basename $@)).so.1.0 $@

$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a.1: %.a.1: %.so.1.0
	ln -sf $(basename $(basename $@)).so.1.0 $@

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a: %: %.1
	ln -sf $@.1 $@

$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a: %: %.1
	ln -sf $@.1 $@

$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a: %: %.1
	ln -sf $@.1 $@

platform.clean:
	rm -f $(PLATFORM_OBJECTS)
	rm -f $(DROMAJO_OBJECTS)
	rm -f $(BSG_PLATFORM_PATH)/lfs.o
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a.1
	rm -f $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a
	rm -f $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a.1
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a.1
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so

libraries.clean: platform.clean
