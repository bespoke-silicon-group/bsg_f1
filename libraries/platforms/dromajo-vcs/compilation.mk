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

# This Makefile fragment defines rules for compilation of the C/C++
# files for running regression tests.

ORANGE=\033[0;33m
RED=\033[0;31m
NC=\033[0m

# This file REQUIRES several variables to be set. They are typically
# set by the Makefile that includes this makefile..
# 
DEFINES    += -UVCS -D_DRAMFS
INCLUDES   += -I$(LIBRARIES_PATH)
INCLUDES   += -I$(BSG_PLATFORM_PATH)/software/include
INCLUDES   += -I$(BLACKPARROT_DIR)/sdk/perch

CXXFLAGS   += $(DEFINES) -march=rv64imafd -mabi=lp64 -mcmodel=medany
CFLAGS     += $(DEFINES) -march=rv64imafd -mabi=lp64 -mcmodel=medany
SFLAGS     += $(DEFINES) -march=rv64imafd -mabi=lp64 -mcmodel=medany 

export RV_CC = $(BLACKPARROT_DIR)/sdk/install/bin/riscv64-unknown-elf-dramfs-gcc
export RV_CXX = $(BLACKPARROT_DIR)/sdk/install/bin/riscv64-unknown-elf-dramfs-g++
export RV_LD = $(RV_CXX)
export RV_AR = $(BLACKPARROT_DIR)/sdk/install/bin/riscv64-unknown-elf-dramfs-ar
export RV_RANLIB = $(BLACKPARROT_DIR)/sdk/install/bin/riscv64-unknown-elf-dramfs-ranlib

# each regression target needs to build its .o from a .c and .h of the
# same name
%.o: %.c
	$(RV_CC) -c -o $@ $< $(INCLUDES) $(CFLAGS) $(CDEFINES)

# ... or a .cpp and .hpp of the same name
%.o: %.cpp
	$(RV_CXX) -c -o $@ $< $(INCLUDES) $(CXXFLAGS) $(CXXDEFINES)

%.o: %.S
	$(RV_CC) -c -o $@ $< $(INCLUDES) $(SFLAGS)

.PRECIOUS: %.o

.PHONY: platform.compilation.clean
platform.compilation.clean:
	rm -rf *.o

compilation.clean: platform.compilation.clean
