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

# RISC-V tools
RV_CC = $(BLACKPARROT_SDK_DIR)/install/bin/riscv64-unknown-elf-dramfs-gcc
RV_CXX = $(BLACKPARROT_SDK_DIR)/install/bin/riscv64-unknown-elf-dramfs-g++
RV_AR = $(BLACKPARROT_SDK_DIR)/install/bin/riscv64-unknown-elf-dramfs-ar
RV_OBJDUMP = $(BLACKPARROT_SDK_DIR)/install/bin/riscv64-unknown-elf-dramfs-objdump

DROMAJO_DIR = $(BLACKPARROT_SDK_DIR)/dromajo

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
LIB_CSOURCES += $(BLACKPARROT_SDK_DIR)/perch/bsg_newlib_intf.c
LIB_CSOURCES += $(BLACKPARROT_SDK_DIR)/perch/emulation.c

LIB_SSOURCES += $(BLACKPARROT_SDK_DIR)/perch/atomics.S
LIB_SSOURCES += $(BLACKPARROT_SDK_DIR)/perch/exception.S
LIB_SSOURCES += $(BLACKPARROT_SDK_DIR)/perch/muldiv.S
LIB_SSOURCES += $(BSG_PLATFORM_PATH)/software/src/crt0.S

# For now, use the noimpl version of the features
include $(LIBRARIES_PATH)/features/dma/noimpl/feature.mk

$(DMA_FEATURE_OBJECTS): INCLUDES += -I$(BSG_PLATFORM_PATH)/software/include
$(DMA_FEATURE_OBJECTS): CFLAGS += -march=rv64imafd -mcmodel=medany -mabi=lp64 -D_BSD_SOURCE -D_XOPEN_SOURCE=500
$(DMA_FEATURE_OBJECTS): CXXFLAGS += -march=rv64imafd -mcmodel=medany -mabi=lp64 -D_BSD_SOURCE -D_XOPEN_SOURCE=500

# Add the riscv-newlib specific includes for the library
LIB_PLATFORM_INCLUDES =  -I$(BSG_PLATFORM_PATH)/software/include
LIB_PLATFORM_INCLUDES += -I$(BLACKPARROT_SDK_DIR)/perch

$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): CC = $(RV_CC)
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): CXX = $(RV_CXX)

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a: AR = $(RV_AR)
$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a: AR = $(RV_AR)
$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a: AR = $(RV_AR)

# Make the litteFS file system
$(BSG_PLATFORM_PATH)/lfs.o: MKLFS = $(BLACKPARROT_SDK_DIR)/install/bin/dramfs_mklfs 128 64
$(BSG_PLATFORM_PATH)/lfs.o:
	$(MKLFS) > $(BSG_PLATFORM_PATH)/lfs.cpp
	$(CXX) $(BSG_PLATFORM_PATH)/lfs.cpp -c -o $@ $(CXXFLAGS) $(INCLUDES)

include $(LIBRARIES_PATH)/features/dma/simulation/dramsim3.mk
include $(LIBRARIES_PATH)/features/dma/simulation/libdmamem.mk

$(LIBRARIES_PATH)/features/dma/simulation/libdmamem.so: INCLUDES :=
$(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so: INCLUDES :=

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
$(DROMAJO_OBJECTS): CXX = g++

PLATFORM_CXXSOURCES += $(BSG_PLATFORM_PATH)/bsg_manycore_simulator.cpp

PLATFORM_OBJECTS += $(patsubst %cpp,%o,$(PLATFORM_CXXSOURCES))
PLATFORM_OBJECTS += $(patsubst %c,%o,$(PLATFORM_CSOURCES))

$(PLATFORM_OBJECTS): INCLUDES := -I$(LIBRARIES_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(EXAMPLES_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_MACHINE_PATH)/notrace/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_PLATFORM_PATH)
$(PLATFORM_OBJECTS): INCLUDES += -I$(VCS_HOME)/linux64/lib/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BSG_MANYCORE_DIR)/testbenches/dpi/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BASEJUMP_STL_DIR)/bsg_test/
$(PLATFORM_OBJECTS): INCLUDES += -I$(BLACKPARROT_SDK_DIR)/dromajo/include
$(PLATFORM_OBJECTS): CFLAGS   := -std=c11 -fPIC -DVCS -D_GNU_SOURCE -DVERILATOR -D_BSD_SOURCE -D_XOPEN_SOURCE=500
$(PLATFORM_OBJECTS): CXXFLAGS := -std=c++11 -fPIC -DVCS -D_GNU_SOURCE -DVERILATOR -D_BSD_SOURCE -D_XOPEN_SOURCE=500
$(PLATFORM_OBJECTS): LDFLAGS  := -fPIC
$(PLATFORM_OBJECTS): CXX = g++

$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: $(DROMAJO_OBJECTS)
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: $(PLATFORM_OBJECTS)
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: $(LIBRARIES_PATH)/features/dma/simulation/libdmamem.so
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: $(LIBRARIES_PATH)/features/dma/simulation/libdramsim3.so
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: LDFLAGS := -fPIC
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: LDFLAGS += -L$(LIBRARIES_PATH)/features/dma/simulation -Wl,-rpath=$(LIBRARIES_PATH)/features/dma/simulation -ldmamem
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: LDFLAGS += -L$(LIBRARIES_PATH)/features/dma/simulation -Wl,-rpath=$(LIBRARIES_PATH)/features/dma/simulation -ldramsim3
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0: LD = $(CXX)
$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1.0:
	$(LD) -shared -Wl,-soname,$(basename $(notdir $@)) -o $@ $(DROMAJO_OBJECTS) $(PLATFORM_OBJECTS) $(LDFLAGS)

$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so.1: %: %.0
	ln -sf $@.0 $@

$(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so: %: %.1
	ln -sf $@.1 $@

platform.clean:
	rm -f $(PLATFORM_OBJECTS)
	rm -f $(DROMAJO_OBJECTS)
	rm -f $(BSG_PLATFORM_PATH)/lfs.o
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a
	rm -f $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a

libraries.clean: platform.clean
