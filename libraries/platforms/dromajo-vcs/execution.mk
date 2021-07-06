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


# All simulations should run with +ntb_random_seed_automatic.
# 
# From the VCS MX User-Guide: +ntb_random_seed_automatic Picks a unique value to
# supply as the first seed used by a testbench. The value is determined by
# combining the time of day, host name and process id. This ensures that no two
# simulations have the same starting seed.
SIM_ARGS += +ntb_random_seed_automatic

# These are the execution rules for the binaries. We can't pass
# C-style arguments through the command line, so instead we specify
# them as the VCS plusarg argument +c_args. Users can specify C-style
# arguments using the C_ARGS make variable.

%.log: % $(BSG_MANYCORE_KERNELS)
	./$< $(SIM_ARGS) +c_args="$(C_ARGS)" 2>&1 | tee $@

vanilla_stats.csv vcache_stats.csv router_stat.csv: % : %.profile.log

%.saif: %.saifgen.log ;

%.vpd: SIM_ARGS += +vpdfile+$(@:.debug.log=.vpd)
%.vpd: %.debug.log ;

%.dve: %.vpd
	$(DVE) -full64 -vpd $< &

.PRECIOUS: %.log

.PHONY: platform.execution.clean %.log %.vpd
platform.execution.clean:
	rm -rf vanilla_stats.csv
	rm -rf infinite_mem_stats.csv
	rm -rf vcache_stats.csv
	rm -rf vanilla_operation_trace.csv
	rm -rf operation_trace.csv
	rm -rf vcache_operation_trace.csv
	rm -rf router_stat.csv
	rm -rf remote_load_trace.csv
	rm -rf vanilla.log
	rm -rf *.vpd
	rm -rf dramsim3.json dramsim3.tag.json dramsim3.txt dramsim3epoch.json

execution.clean: platform.execution.clean
