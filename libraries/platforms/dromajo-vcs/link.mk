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
# aws-vcs binaries

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

# VHEADERS must be compiled before VSOURCES.
VDEFINES += BSG_MACHINE_ORIGIN_X_CORD=$(BSG_MACHINE_ORIGIN_COORD_X)
VDEFINES += BSG_MACHINE_ORIGIN_Y_CORD=$(BSG_MACHINE_ORIGIN_COORD_Y)
VDEFINES += HOST_MODULE_PATH=replicant_tb_top
VDEFINES += BSG_MACHINE_DRAMSIM3_PKG=$(BSG_MACHINE_MEM_DRAMSIM3_PKG)
VLOGAN_SOURCES  += $(VHEADERS) $(VSOURCES)
VLOGAN_INCLUDES += $(foreach inc,$(VINCLUDES),+incdir+"$(inc)")
VLOGAN_DEFINES  += $(foreach def,$(VDEFINES),+define+"$(def)")
VLOGAN_VFLAGS   += -timescale=1ps/1ps
VLOGAN_VFLAGS   += -sverilog
VLOGAN_VFLAGS   += +systemverilogext+.svh +systemverilogext+.sv
VLOGAN_VFLAGS   += +libext+.sv +libext+.v +libext+.vh +libext+.svh
VLOGAN_VFLAGS   += -full64 -lca -assert svaext
VLOGAN_VFLAGS   += -undef_vcs_macro
VLOGAN_FLAGS    = $(VLOGAN_VFLAGS) $(VLOGAN_DEFINES) $(VLOGAN_INCLUDES) $(VLOGAN_SOURCES)

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM):
	mkdir -p $@

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.setup: | $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)
	echo "replicant_tb_top : $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top/64" >> $@;

# The SAIF-generation simulation and fast simulation (exec) turns off the profilers using macros
# so it must be compiled into a separate library.
$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.saifgen: | $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)
	echo "replicant_tb_top : $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/64" >> $@;

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.exec: | $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)
	echo "replicant_tb_top : $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/64" >> $@;

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top/AN.DB: $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.setup $(VLOGAN_SOURCES)
	SYNOPSYS_SIM_SETUP=$< vlogan -work replicant_tb_top $(VLOGAN_FLAGS) -l $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vlogan.log

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/AN.DB: VDEFINES += BSG_MACHINE_DISABLE_VCORE_PROFILING
$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/AN.DB: VDEFINES += BSG_MACHINE_DISABLE_CACHE_PROFILING
$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/AN.DB: VDEFINES += BSG_MACHINE_DISABLE_ROUTER_PROFILING

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB: VDEFINES += BSG_MACHINE_ENABLE_SAIF
$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB: $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.saifgen $(VLOGAN_SOURCES)
	SYNOPSYS_SIM_SETUP=$< vlogan -work replicant_tb_top -debug_access+pp $(VLOGAN_FLAGS) -l $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vlogan.saifgen.log

$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/AN.DB: $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.exec $(VLOGAN_SOURCES)
	SYNOPSYS_SIM_SETUP=$< vlogan -work replicant_tb_top $(VLOGAN_FLAGS) -l $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vlogan.exec.log

# libbsg_manycore_platform will be compiled in $(BSG_PLATFORM_PATH)
LDFLAGS += -L$(BSG_PLATFORM_PATH) -Wl,-rpath=$(BSG_PLATFORM_PATH)
LDFLAGS += -lbsg_manycore_platform -lm

VCS_LDFLAGS += $(foreach def,$(LDFLAGS),-LDFLAGS "$(def)")
VCS_VFLAGS  += -M -L -ntb_opts tb_timescale=1ps/1ps -lca
VCS_VFLAGS  += -timescale=1ps/1ps -sverilog -full64 -licqueue -q
VCS_VFLAGS  += +warn=noLCA_FEATURES_ENABLED
VCS_VFLAGS  += +warn=noMC-FCNAFTMI
VCS_VFLAGS  += +lint=all,TFIPC-L,noSVA-UA,noSVA-NSVU,noVCDE,noSVA-AECASR
VCS_FLAGS   = $(VCS_LDFLAGS) $(VCS_VFLAGS)

TEST_CSOURCES   += $(filter %.c,$(TEST_SOURCES))
TEST_CXXSOURCES += $(filter %.cpp,$(TEST_SOURCES))
TEST_OBJECTS    += $(TEST_CXXSOURCES:.cpp=.o)
TEST_OBJECTS    += $(TEST_CSOURCES:.c=.o)

$(BSG_PLATFORM_PATH)/test.riscv: CC = $(RV_CC)
$(BSG_PLATFORM_PATH)/test.riscv: CXX = $(RV_CXX)
$(BSG_PLATFORM_PATH)/test.riscv: $(TEST_OBJECTS)
$(BSG_PLATFORM_PATH)/test.riscv: $(BSG_PLATFORM_PATH)/lfs.o
$(BSG_PLATFORM_PATH)/test.riscv: $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a
$(BSG_PLATFORM_PATH)/test.riscv: $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a
$(BSG_PLATFORM_PATH)/test.riscv: $(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a
$(BSG_PLATFORM_PATH)/test.riscv: LDFLAGS := -T$(BLACKPARROT_SDK_DIR)/linker/riscv.ld -L$(BSG_PLATFORM_PATH) -static -nostartfiles -lstdc++ -lbsg_manycore_runtime -lbsg_manycore_regression
$(BSG_PLATFORM_PATH)/test.riscv: LD = $(CXX)
$(BSG_PLATFORM_PATH)/test.riscv:
	$(LD) -D_DRAMFS -o $@ $(BSG_PLATFORM_PATH)/software/src/crt0.o $(BSG_PLATFORM_PATH)/lfs.o $< $(LDFLAGS)
	cp $@ main.elf

# VCS Generates an executable file by linking the TEST_OBJECTS with
# the the VCS work libraries for the design, and the runtime shared
# libraries

%.saifgen %.debug: VCS_VFLAGS += -debug_pp
%.debug: VCS_VFLAGS += +plusarg_save +vcs+vcdpluson +vcs+vcdplusmemon +memcbk

%.debug %.profile: $(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so $(BSG_PLATFORM_PATH)/test.riscv $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top/AN.DB $(PLATFORM_OBJECTS)
	SYNOPSYS_SIM_SETUP=$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.setup \
	vcs -top replicant_tb_top $(PLATFORM_OBJECTS) $(VCS_FLAGS) -Mdirectory=$@.tmp -l $@.vcs.log -o $@

%.saifgen: $(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so $(BSG_PLATFORM_PATH)/test.riscv $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB $(PLATFORM_OBJECTS)
	SYNOPSYS_SIM_SETUP=$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.saifgen \
	vcs -top replicant_tb_top $(PLATFORM_OBJECTS) $(VCS_FLAGS) -Mdirectory=$@.tmp -l $@.vcs.log -o $@

%.exec: $(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so $(BSG_PLATFORM_PATH)/test.riscv $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/AN.DB $(PLATFORM_OBJECTS)
	SYNOPSYS_SIM_SETUP=$(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/synopsys_sim.exec \
	vcs -top replicant_tb_top $(PLATFORM_OBJECTS) $(VCS_FLAGS) -Mdirectory=$@.tmp -l $@.vcs.log -o $@

.PRECIOUS:%.debug %.profile %.saifgen %.exec

# When running recursive regression, make is launched in independent,
# non-communicating parallel processes that try to build these objects
# in parallel. That is no-bueno. We define REGRESSION_PREBUILD so that
# regression tests can build them before launching parallel
# compilation and execution
REGRESSION_PREBUILD += $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top/AN.DB
REGRESSION_PREBUILD += $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_exec/AN.DB
REGRESSION_PREBUILD += $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/vcs_simlibs/replicant_tb_top_saifgen/AN.DB
REGRESSION_PREBUILD += $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.a
REGRESSION_PREBUILD += $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a
REGRESSION_PREBUILD += $(BSG_PLATFORM_PATH)/libbsg_manycore_regression.a
REGRESSION_PREBUILD += $(BSG_PLATFORM_PATH)/libbsg_manycore_platform.so

.PHONY: platform.link.clean
platform.link.clean:
	rm -rf $(BSG_MACHINE_PATH)/$(BSG_PLATFORM)/
	rm -rf $(BSG_PLATFORM_PATH)/msg_config
	rm -rf ucli.key
	rm -rf .cxl* *.jou
	rm -rf *.daidir *.tmp
	rm -rf *.jou
	rm -rf *.vcs.log
	rm -rf vc_hdrs.h
	rm -rf *.debug *.profile *.saifgen *.exec
	rm -rf *.elf
	rm -rf $(BSG_PLATFORM_PATH)/*.riscv

link.clean: platform.link.clean ;

