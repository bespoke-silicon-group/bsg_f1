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

# This Makefile fragment defines the rules that are used for executing
# applications on HammerBlade Platforms


# All simulations should run with +ntb_random_seed_automatic.
# 
# From the VCS MX User-Guide: +ntb_random_seed_automatic Picks a unique value to
# supply as the first seed used by a testbench. The value is determined by
# combining the time of day, host name and process id. This ensures that no two
# simulations have the same starting seed.
SIM_ARGS += +ntb_random_seed_automatic +2state

%.debug.log: %.vpd ;

# OK, here's where things get complicated. 

# Each regression that generates a waveform generates it in the .<test_name>
# directory, named vcdplus.vpd. To generate a waveform we must provide the
# following arguments to the simulation through SIM_ARGS.
.%/vcdplus.vpd: SIM_ARGS += +vcs+vcdpluson +vcs+vcdplusmemon +memcbk


# Generating a waveform requires a simulation binary compiled with different VCS
# flags. We call this binary <test_name>.debug, and its build rule is defined in
# link.mk.
#
# UNIFIED_TESTS all use test_loader.debug. INDEPENDENT_TESTS use their own
# top-level. Encode dependencies for each differently.
#
# The ORDER of these dependencies are important because the actual rule uses $<
# to get the name of the executable.
$(UNIFIED_TESTS:%=.%/vcdplus.vpd): test_loader.debug
$(INDEPENDENT_TESTS:%=.%/vcdplus.vpd): .%/vcdplus.vpd : %.debug
# Each regression test can define its dependencies in <test_name>.rule, but we
# must satisfy that rule before generating the .vpd file
$(REGRESSION_TESTS:%=.%/vcdplus.vpd): .%/vcdplus.vpd : %.rule

# Normal cosimulation requires a simulation binary without waveform flags (it
# executes faster...). We call this binary <test_name>, and its build rule is
# defined in link.mk.
#
# UNIFIED_TESTS all use test_loader. INDEPENDENT_TESTS use their own top-level
# that corresponds to their <test_name>. Encode dependencies for each
# differently.
#
# The ORDER of these dependencies are important because the actual rule uses $<
# to get the name of the executable.
$(UNIFIED_TESTS:%=.%/vanilla_stats.csv): test_loader
$(INDEPENDENT_TESTS:%=.%/vanilla_stats.csv): .%/vanilla_stats.csv : %
# Each regression test can define its dependencies in <test_name>.rule, but we
# must satisfy that rule before generating the .csv file
$(REGRESSION_TESTS:%=.%/vanilla_stats.csv): .%/vanilla_stats.csv : %.rule

# These are the execution rules for the binaries. We define TEST_NAME so that it
# can be used in C_ARGS, and LOG_NAME so that we can write a log. If a waveform
# is being generated, then it writes to <test_name>.debug.log, otherwise
# <test_name>.log. Finally, depend on <test_name>.rule so that we rebuild the
# RISC-V binaries.

.%/vcdplus.vpd .%/vanilla_stats.csv: TEST_NAME=$(@:.%/$(notdir $@)=%)
.%/vcdplus.vpd: LOG_NAME=$(subst ..,.,$(TEST_NAME).debug.log)
.%/vanilla_stats.csv: LOG_NAME=$(subst ..,.,$(TEST_NAME).log)

.%/vcdplus.vpd:
	mkdir -p .$(TEST_NAME)
	cd .$(TEST_NAME) && \
	../$< $(SIM_ARGS) +c_args="$(C_ARGS)" 2>&1 | tee $(LOG_NAME)
	@mv .$(TEST_NAME)/$(LOG_NAME) $(LOG_NAME)

.%/vanilla_stats.csv: 
	mkdir -p .$(TEST_NAME)
	cd .$(TEST_NAME) && \
	../$< $(SIM_ARGS) +c_args="$(C_ARGS)" 2>&1 | tee $(LOG_NAME)
	@mv .$(TEST_NAME)/$(LOG_NAME) $(LOG_NAME)

# %.debug.log is just an alias for %.vpd
%.debug.log: %.vpd ;
# Running simulation generates <test_name>/vcdplus.vpd. Move it to
# <test_name>.vpd

# set the the name for MEMSYS_STATS
ifneq ($(filter e_vcache_%, $(CL_MANYCORE_MEM_CFG)),)
MEMSYS_STATS := vcache_stats
endif

ifneq ($(filter e_infinite_mem, $(CL_MANYCORE_MEM_CFG)),)
MEMSYS_STATS := infinite_mem_stats
endif

%.vpd: .%/vcdplus.vpd %.vanilla_stats.csv %.$(MEMSYS_STATS).csv
	@mv $< $@

# Vcache stats are generated by default, at the same time as
# vanilla_stats.
.%/$(MEMSYS_STATS).csv: .%/vanilla_stats.csv ;
%.$(MEMSYS_STATS).csv: .%/$(MEMSYS_STATS).csv
	@mv $< $@ 

# %.log is an alias for %.vanilla_stats.csv. 
%.log: %.vanilla_stats.csv ;
# vanilla_stats.csv is generated in .<test_name>, so we move it out and into a
# name-specific file.
%.vanilla_stats.csv: .%/vanilla_stats.csv %.$(MEMSYS_STATS).csv
	@mv $< $@

# .%/vanilla_operation_trace.csv, .%/vcache_operation_trace.csv 
# and .%/vanilla.log are generated by running a simulation
# with the +trace flag. They are generated into the .<test_name>
# directory, so we copy them out into %.vanilla.log and
# %.vanilla_operation_trace.csv to make them test-specific.
.%/operation_trace.csv .%/vanilla_operation_trace.csv .%/vcache_operation_trace.csv .%/vanilla.log: SIM_ARGS += +trace
.%/operation_trace.csv .%/vanilla_operation_trace.csv .%/vcache_operation_trace.csv .%/vanilla.log: %.vanilla_stats.csv ;

%.vanilla_operation_trace.csv: .%/vanilla_operation_trace.csv
	@mv $< $@

%.vcache_operation_trace.csv: .%/vcache_operation_trace.csv
	@mv $< $@

# %.operation_trace.csv generates both vanilla_operation_trace.csv
# and vcache_operation_trace.csv
%.operation_trace.csv: %.vanilla_operation_trace.csv %.vcache_operation_trace.csv ;

%.vanilla.log: .%/vanilla.log
	@mv $< $@

%.dve: %.vpd
	$(DVE) -full64 -vpd $< &

# Do Not remove intermediate targets 
.PRECIOUS: %.vanilla_stats.csv
.PRECIOUS: %.operation_trace.csv %.vanilla_operation_trace.csv
.PRECIOUS: %.vcache_operation_trace.csv %.vcache_stats.csv
.PRECIOUS: %.infinite_mem_stats.csv %.vanilla.log

.PHONY: platform.execution.clean
platform.execution.clean:
	rm -rf vanilla_stats.csv *.vanilla_stats.csv
	rm -rf infinite_mem_stats.csv *.infinite_mem_stats.csv
	rm -rf vcache_stats.csv *.vcache_stats.csv
	rm -rf vanilla_operation_trace.csv *.vanilla_operation_trace.csv
	rm -rf operation_trace.csv *.operation_trace.csv
	rm -rf vcache_operation_trace.csv *.vcache_operation_trace.csv
	rm -rf vanilla.log *.vanilla.log
	rm -rf *.vpd
	rm -rf .test*

execution.clean: platform.execution.clean
