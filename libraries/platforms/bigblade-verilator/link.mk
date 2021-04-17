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

# This Makefile fragment defines all of the rules for linking
# bigblade-verilator binaries

ORANGE=\033[0;33m
RED=\033[0;31m
NC=\033[0m

# This file REQUIRES several variables to be set. They are typically set by the
# Makefile that includes this fragment...

# BSG_PLATFORM_PATH: The path to the testbenches folder in BSG F1
ifndef BSG_PLATFORM_PATH
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: BSG_PLATFORM_PATH is not defined$(NC)"))
endif

# hardware.mk is the file list for the simulation RTL. It includes the
# platform specific hardware.mk file.
include $(HARDWARE_PATH)/hardware.mk

# libraries.mk defines how to build libbsg_manycore_runtime.so, which is
# pre-linked against all other simulation binaries.
include $(LIBRARIES_PATH)/libraries.mk

# Generic Verilator source files that are compiiled into libmachine.so
LIBMACHINE_CXXSRCS := verilated.cpp verilated_vcd_c.cpp verilated_dpi.cpp
LIBMACHINE_OBJS += $(LIBMACHINE_CXXSRCS:%.cpp=%.o)
LIBMACHINE_OBJECTS = $(LIBMACHINE_OBJS:%.o=$(MACHINES_PATH)/%.o)

$(LIBMACHINE_OBJECTS): DEFINES := -DVL_PRINTF=printf
$(LIBMACHINE_OBJECTS): DEFINES += -DVM_SC=0
$(LIBMACHINE_OBJECTS): DEFINES += -DVM_TRACE=0
$(LIBMACHINE_OBJECTS): INCLUDES := -I$(VERILATOR_ROOT)/include
$(LIBMACHINE_OBJECTS): INCLUDES += -I$(VERILATOR_ROOT)/include/vltstd
$(LIBMACHINE_OBJECTS): INCLUDES += -I$(BSG_MACHINE_PATH)

$(LIBMACHINE_OBJECTS): CFLAGS    += -std=c11 -fPIC $(INCLUDES) $(DEFINES)
$(LIBMACHINE_OBJECTS): CXXFLAGS  += -std=c++11 -fPIC $(INCLUDES) $(DEFINES)
$(LIBMACHINE_OBJECTS): $(MACHINES_PATH)/%.o: $(VERILATOR_ROOT)/include/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

# bsg_manycore_simulator.cpp is the interface between
# libbsg_manycore_runtime.so and libmachine.so that hides the
# implementation of the Vmanycore_tb_top and allows
# libbsg_manycore_runtime to be compiled independently from the
# machine.
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: INCLUDES := -I$(BSG_PLATFORM_PATH)
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: INCLUDES += -I$(BSG_MACHINE_PATH)/notrace
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: INCLUDES += -I$(BSG_MACHINE_PATH)
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: INCLUDES += -I$(BASEJUMP_STL_DIR)/bsg_test
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: INCLUDES += -I$(VERILATOR_ROOT)/include
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: INCLUDES += -I$(VERILATOR_ROOT)/include/vltstd
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: CXXFLAGS := -std=c++11 -fPIC $(INCLUDES)
$(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o: $(BSG_MACHINE_PATH)/V$(BSG_DESIGN_TOP)__ALL.a

# VHEADERS must be compiled before VSOURCES.
VDEFINES += BSG_MACHINE_ORIGIN_X_CORD=$(BSG_MACHINE_ORIGIN_COORD_X)
VDEFINES += BSG_MACHINE_ORIGIN_Y_CORD=$(BSG_MACHINE_ORIGIN_COORD_Y)
VDEFINES += HOST_MODULE_PATH=replicant_tb_top
VDEFINES += BSG_MACHINE_DRAMSIM3_PKG=$(BSG_MACHINE_MEM_DRAMSIM3_PKG)
VCFLAGS = -fPIC -Wno-format-extra-args
VERILATOR_CFLAGS    += $(foreach vcf,$(VCFLAGS),-CFLAGS "$(vcf)")
VERILATOR_VINCLUDES += $(foreach inc,$(VINCLUDES),+incdir+"$(inc)")
VERILATOR_VDEFINES  += $(foreach def,$(VDEFINES),+define+"$(def)")
VERILATOR_VFLAGS = $(VERILATOR_VINCLUDES) $(VERILATOR_VDEFINES)
VERILATOR_VFLAGS += -Wno-widthconcat -Wno-unoptflat -Wno-lint
VERILATOR_VFLAGS += --assert
# These enable verilator tracing
# VERILATOR_VFLAGS += --trace --trace-structs

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM):
	mkdir -p $@

# libbsg_manycore_runtime will be compiled in $(BSG_PLATFORM_PATH)
LDFLAGS += -lbsg_manycore_runtime -lm
LDFLAGS += -L$(BSG_PLATFORM_PATH) -Wl,-rpath=$(BSG_PLATFORM_PATH)
LDFLAGS += -lmachine -L$(BSG_MACHINE_PATH) -Wl,-rpath=$(BSG_MACHINE_PATH)

TEST_CSOURCES   += $(filter %.c,$(TEST_SOURCES))
TEST_CXXSOURCES += $(filter %.cpp,$(TEST_SOURCES))
TEST_OBJECTS    += $(TEST_CXXSOURCES:.cpp=.o)
TEST_OBJECTS    += $(TEST_CSOURCES:.c=.o)

$(BSG_MACHINE_PATH)/V$(BSG_DESIGN_TOP)__ALL.a: $(VHEADERS) $(VSOURCES)
	$(info BSG_INFO: Running verilator)
	@$(VERILATOR) -Mdir $(dir $@) --build --cc $(VERILATOR_CFLAGS) $(VERILATOR_VFLAGS) $^ --top-module $(BSG_DESIGN_TOP)

# libmachine.so is the machine-specific shared object file that implements the
# Verilator simulation. It is generated by compiling the archive file (above),
# verilator C++ files, and the Machine/Manycore interface
# bsg_manycore_verilator.o. It is linked with libbsg_manycore_runtime.so at
# executable compile time.
$(BSG_MACHINE_PATH)/libmachine.so: LD = $(CXX)
$(BSG_MACHINE_PATH)/libmachine.so: $(LIBMACHINE_OBJECTS)
$(BSG_MACHINE_PATH)/libmachine.so: $(BSG_PLATFORM_PATH)/bsg_manycore_simulator.o
$(BSG_MACHINE_PATH)/libmachine.so: $(BSG_MACHINE_PATH)/V$(BSG_DESIGN_TOP)__ALL.a
	$(LD) -shared -Wl,--whole-archive,-soname,$@ -o $@ $^ -Wl,--no-whole-archive

%.exec: $(TEST_OBJECTS) $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so
	g++ -std=c++11 -o $@ $(LDFLAGS) $(TEST_OBJECTS)

.PRECIOUS:%.debug %.profile %.saifgen %.exec

# When running recursive regression, make is launched in independent,
# non-communicating parallel processes that try to build these objects
# in parallel. That is no-bueno. We define REGRESSION_PREBUILD so that
# regression tests can build them before launching parallel
# compilation and execution
REGRESSION_PREBUILD += $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so
REGRESSION_PREBUILD += $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so
REGRESSION_PREBUILD += $(BSG_MACHINE_PATH)/libmachine.so

.PHONY: platform.link.clean
platform.link.clean:
	rm -rf $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/
	rm -rf *.debug *.profile *.saifgen *.exec

link.clean: platform.link.clean ;

