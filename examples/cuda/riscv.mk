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

# TODO: Makefile comment
ORANGE=\033[0;33m
RED=\033[0;31m
NC=\033[0m

################################################################################
# Paths
################################################################################
_REPO_ROOT ?= $(shell git rev-parse --show-toplevel)
-include $(_REPO_ROOT)/environment.mk
-include $(BSG_MACHINE_PATH)/Makefile.machine.include

BSG_MANYCORE_SPMD_PATH = $(BSG_MANYCORE_DIR)/software/spmd/
BSG_MANYCORE_CUDALITE_PATH = $(BSG_MANYCORE_SPMD_PATH)/bsg_cuda_lite_runtime/
BSG_MANYCORE_CUDALITE_MAIN_PATH = $(BSG_MANYCORE_CUDALITE_PATH)/main

BSG_MANYCORE_LIB_PATH    = $(BSG_MANYCORE_DIR)/software/bsg_manycore_lib
BSG_MANYCORE_COMMON_PATH = $(BSG_MANYCORE_SPMD_PATH)/common/

RISCV_TOOLS_PATH := $(BSG_MANYCORE_DIR)/software/riscv-tools/
RISCV_GNU_PATH   := $(RISCV_TOOLS_PATH)/riscv-install
RISCV_LLVM_PATH  := $(RISCV_TOOLS_PATH)/llvm/llvm-install

################################################################################
# Include RISC-V Tool Configuration
################################################################################

RISCV_LINK_GEN := $(BSG_MANYCORE_DIR)/software/py/bsg_manycore_link_gen.py

# These flags are not supported by clang
RISCV_GNU_FLAGS = -frerun-cse-after-loop -fweb -frename-registers -mtune=bsg_vanilla_2020

RISCV_GCC        ?= $(RISCV_GNU_PATH)/bin/riscv32-unknown-elf-dramfs-gcc $(RISCV_GNU_FLAGS)
RISCV_GXX        ?= $(RISCV_GNU_PATH)/bin/riscv32-unknown-elf-dramfs-g++ $(RISCV_GNU_FLAGS)
RISCV_ELF2HEX    ?= LD_LIBRARY_PATH=$(RISCV_GNU_PATH)/lib $(RISCV_GNU_PATH)/bin/elf2hex
RISCV_OBJCOPY    ?= $(RISCV_GNU_PATH)/bin/riscv32-unknown-elf-dramfs-objcopy
RISCV_AR         ?= $(RISCV_GNU_PATH)/bin/riscv32-unknown-elf-dramfs-ar
RISCV_OBJDUMP    ?= $(RISCV_GNU_PATH)/bin/riscv32-unknown-elf-dramfs-objdump
RISCV_LINK       ?= $(RISCV_GCC) -t -T $(LINK_SCRIPT) $(RISCV_LDFLAGS)
RISCV_LD         ?= $(RISCV_GCC)

RISCV_CLANG_ABI        = ilp32f
RISCV_CLANG_CCPPFLAGS += --target=riscv32 -mabi=$(RISCV_CLANG_ABI) -march=riscv32imaf -mtune=hb-rv32
RISCV_CLANG_CXXFLAGS  += --sysroot=$(RISCV_GNU_PATH)/riscv32-unknown-elf-dramfs
RISCV_CLANG_CXXFLAGS  += -I$(RISCV_GNU_PATH)/riscv32-unknown-elf-dramfs/include/c++/9.2.0
RISCV_CLANG_CXXFLAGS  += -I$(RISCV_GNU_PATH)/riscv32-unknown-elf-dramfs/include/c++/9.2.0/riscv32-unknown-elf-dramfs  

RISCV_CLANG       ?= $(RISCV_LLVM_PATH)/bin/clang $(RISCV_CLANG_CFLAGS) $(RISCV_CLANG_CCPPFLAGS)
RISCV_CLANGXX     ?= $(RISCV_LLVM_PATH)/bin/clang++ $(RISCV_CLANG_CXXFLAGS) $(RISCV_CLANG_CCPPFLAGS)
RISCV_LLVM_OPT    ?= $(RISCV_LLVM_PATH)/bin/opt
RISCV_LLVM_LLC    ?= $(RISCV_LLVM_PATH)/bin/llc
RISCV_LLVM_LIB    ?= $(RISCV_LLVM_PATH)/lib

# Set the default RISC-V Compilers. To override these globally set
# RISCV_CXX = $(RISCV_CLANGXX), etc. This can also be done on a
# per-object basis. For example, foo.rvo: RISCV_CXX=$(RISCV_CLANGXX)
RISCV_CXX ?= $(RISCV_GXX)
RISCV_CC  ?= $(RISCV_GCC)

################################################################################
# C/C++ Compilation Flags
#
# All RISCV C/C++ compilation variables simply have RISCV_* appended.
################################################################################
RISCV_OPT_LEVEL   ?= -O2
RISCV_ARCH_OP     := rv32imaf

# CCPPFLAGS are common between GCC and G++
RISCV_CCPPFLAGS += $(RISCV_OPT_LEVEL)
RISCV_CCPPFLAGS += -march=$(RISCV_ARCH_OP)
RISCV_CCPPFLAGS += -g
RISCV_CCPPFLAGS += -static
RISCV_CCPPFLAGS += -ffast-math
RISCV_CCPPFLAGS += -fno-common
RISCV_CCPPFLAGS += -ffp-contract=off

RISCV_CFLAGS   += -std=gnu99 $(RISCV_CCPPFLAGS)
RISCV_CXXFLAGS += -std=c++11 $(RISCV_CCPPFLAGS)
RISCV_CXXFLAGS += -fno-threadsafe-statics

RISCV_INCLUDES += -I$(BSG_MANYCORE_COMMON_PATH)
RISCV_INCLUDES += -I$(BSG_MANYCORE_DIR)/software/bsg_manycore_lib

# TODO: Fail if bsg_tiles_X/Y are not set
RISCV_DEFINES += -Dbsg_global_X=$(BSG_MACHINE_GLOBAL_X)
RISCV_DEFINES += -Dbsg_global_Y=$(BSG_MACHINE_GLOBAL_Y)
RISCV_DEFINES += -Dbsg_group_size=$(BSG_MACHINE_POD_TILES)
RISCV_DEFINES += -Dbsg_pods_X=$(BSG_MACHINE_PODS_X)
RISCV_DEFINES += -Dbsg_pods_Y=$(BSG_MACHINE_PODS_Y)
RISCV_DEFINES += -DIO_X_INDEX=$(BSG_MACHINE_HOST_X_CORD)
RISCV_DEFINES += -DIO_Y_INDEX=$(BSG_MACHINE_HOST_Y_CORD)
RISCV_DEFINES += -DPREALLOCATE=0
RISCV_DEFINES += -DHOST_DEBUG=0

# We build and name a machine-specific crt.rvo because it's REALLY
# difficult to figure out why your program/cosimulation is hanging
# when the wrong link script was used during linking
crt.rvo: $(BSG_MANYCORE_COMMON_PATH)/crt.S
	$(RISCV_GCC) $(RISCV_CFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) -c $< -o $@ |& tee $*.comp.log

# We compile these locally so that we don't interfere with the files in
# $(BSG_MANYCORE_LIB_PATH).
# BSG Manycore Library Objects
LIBBSG_MANYCORE_OBJECTS  += bsg_set_tile_x_y.rvo
LIBBSG_MANYCORE_OBJECTS  += bsg_tile_config_vars.rvo
LIBBSG_MANYCORE_OBJECTS  += bsg_printf.rvo

$(LIBBSG_MANYCORE_OBJECTS) main.rvo: RISCV_CXX = $(RISCV_GCC)

$(LIBBSG_MANYCORE_OBJECTS): %.rvo:$(BSG_MANYCORE_LIB_PATH)/%.c
	$(RISCV_GCC) $(RISCV_CFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) -c $< -o $@

main.rvo: $(BSG_MANYCORE_CUDALITE_MAIN_PATH)/main.c
	$(RISCV_GCC) $(RISCV_CFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) -c $< -o $@

%.rvo: %.c
	$(RISCV_CC) $(RISCV_CFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) -c $< -o $@ |& tee $*.gcc.log

%.rvo: %.cpp
	$(RISCV_CXX) $(RISCV_CXXFLAGS) $(RISCV_DEFINES) $(RISCV_INCLUDES) -c $< -o $@ |& tee $*.gcc.log

kernel.compile.clean:
	rm -rf *.rvo *.a

.PRECIOUS: %.rvo

################################################################################
# Linker Flow
################################################################################

# ELF File Parameters
# Default .data section location; LOCAL=>DMEM, SHARED=>DRAM.
BSG_ELF_DEFAULT_DATA_LOC ?= LOCAL

BSG_ELF_OFF_CHIP_MEM := $(BSG_MACHINE_DRAM_INCLUDED)

# Total addressable DRAM size (in 32-bit WORDS, and SIZE bytes)
BSG_ELF_DRAM_WORDS := $(shell expr $(BSG_MACHINE_DRAM_BANK_SIZE_WORDS) \* $(BSG_MACHINE_GLOBAL_X))
BSG_ELF_DRAM_SIZE := $(shell expr $(BSG_ELF_DRAM_WORDS) \* 4)

# Victim Cache Set Size (in 32-bit WORDS and SIZE bytes)
_BSG_ELF_VCACHE_SET_WORDS := $(shell expr $(BSG_MACHINE_VCACHE_WAY) \* $(BSG_MACHINE_VCACHE_BLOCK_SIZE_WORDS))
BSG_ELF_VCACHE_SET_SIZE := $(shell expr $(_BSG_ELF_VCACHE_SET_WORDS) \* 4)

# Victim Cache Column Size (in 32-bit WORDS and SIZE bytes)
_BSG_ELF_VCACHE_COLUMN_WORDS := $(shell expr $(BSG_MACHINE_VCACHE_SET) \* $(_BSG_ELF_VCACHE_SET_WORDS))
BSG_ELF_VCACHE_COLUMN_SIZE := $(shell expr $(_BSG_ELF_VCACHE_COLUMN_WORDS) \* 4)

# Victim Cache Total Size (in 32-bit WORDS, and SIZE BYTES)
_BSG_ELF_VCACHE_MANYCORE_WORDS ?= $(shell expr $(BSG_MACHINE_GLOBAL_X) \* $(_BSG_ELF_VCACHE_COLUMN_WORDS))
BSG_ELF_VCACHE_MANYCORE_SIZE := $(shell expr $(_BSG_ELF_VCACHE_MANYCORE_WORDS) \* 4)

# Compute the ELF Stack Pointer Location.  
ifeq ($(BSG_ELF_DEFAULT_DATA_LOC), LOCAL)
# If the .data segment is in DMEM (LOCAL) then put it at the top of DMEM. (This is the typical case)
BSG_ELF_STACK_PTR ?= 0x00000ffc
else
  # EVA Offset in DRAM
  BSG_ELF_DRAM_EVA_OFFSET = 0x80000000

  ifeq ($(BSG_ELF_OFF_CHIP_MEM), 1)
  # Otherwise, use the top of DRAM (if present),
  _BSG_ELF_DRAM_LIMIT = $(shell expr $(BSG_ELF_DRAM_EVA_OFFSET) + $(BSG_ELF_DRAM_SIZE))
  else
  # Or the Victim Cache address space (if DRAM is disabled/not present).
  _BSG_ELF_DRAM_LIMIT = $(shell expr $(BSG_ELF_DRAM_EVA_OFFSET) + $(BSG_ELF_VCACHE_MANYCORE_SIZE))
  endif
# Finally, Subtract 4 from the maximum memory space address
BSG_ELF_STACK_PTR = $(shell expr $(_BSG_ELF_DRAM_LIMIT) - 4)
endif

# Linker script generation parameters
ifeq ($(BSG_ELF_OFF_CHIP_MEM), 1)
  ifeq ($(BSG_ELF_DEFAULT_DATA_LOC), LOCAL)
    LINK_GEN_OPTS ?= --default_data_loc=dmem --dram_size=$(BSG_ELF_DRAM_SIZE) --sp=$(BSG_ELF_STACK_PTR)
  else ifeq ($(BSG_ELF_DEFAULT_DATA_LOC), SHARED)
    LINK_GEN_OPTS ?= --default_data_loc=dram --dram_size=$(BSG_ELF_DRAM_SIZE) --sp=$(BSG_ELF_STACK_PTR)
  else
    $(error Invalid BSG_ELF_DEFAULT_DATA_LOC = $(BSG_ELF_DEFAULT_DATA_LOC); Only LOCAL and SHARED are valid)
  endif

  LINK_GEN_OPTS += --imem_size=0x01000000 # 16MB
else ifeq ($(BSG_ELF_OFF_CHIP_MEM), 0)
  ifeq ($(BSG_ELF_DEFAULT_DATA_LOC), LOCAL)
    LINK_GEN_OPTS ?= --default_data_loc=dmem --dram_size=$(BSG_ELF_VCACHE_SIZE) --sp=$(BSG_ELF_STACK_PTR)
  else ifeq ($(BSG_ELF_DEFAULT_DATA_LOC), SHARED)
    LINK_GEN_OPTS ?= --default_data_loc=dram --dram_size=$(BSG_ELF_VCACHE_SIZE) --sp=$(BSG_ELF_STACK_PTR)
  else
    $(error Invalid BSG_ELF_DEFAULT_DATA_LOC = $(BSG_ELF_DEFAULT_DATA_LOC); Only LOCAL and SHARED are valid)
  endif

  LINK_GEN_OPTS += --imem_size=0x00008000 # 32KB
else
  $(error Invalid BSG_ELF_OFF_CHIP_MEM = $(BSG_ELF_OFF_CHIP_MEM); Only 0 and 1 are valid)
endif

RISCV_LINK_SCRIPT ?= bsg_link.ld
$(RISCV_LINK_SCRIPT): $(RISCV_LINK_GEN)
	$(RISCV_LINK_GEN) $(LINK_GEN_OPTS) --out=$@

# Link commands and definitions

RISCV_LDFLAGS += -Wl,--defsym,_bsg_elf_dram_size=$(BSG_ELF_DRAM_SIZE)
RISCV_LDFLAGS += -Wl,--defsym,_bsg_elf_vcache_size=$(BSG_ELF_VCACHE_MANYCORE_SIZE)
RISCV_LDFLAGS += -Wl,--defsym,_bsg_elf_stack_ptr=$(BSG_ELF_STACK_PTR)

RISCV_LDFLAGS += -nostdlib
RISCV_LDFLAGS += -march=$(RISCV_ARCH_OP)
RISCV_LDFLAGS += -nostartfiles
RISCV_LDFLAGS += -ffast-math
RISCV_LDFLAGS += -lc
RISCV_LDFLAGS += -lm
RISCV_LDFLAGS += -lgcc

# TODO: temporary fix to solve this problem: https://stackoverflow.com/questions/56518056/risc-v-linker-throwing-sections-lma-overlap-error-despite-lmas-belonging-to-dif
RISCV_LDFLAGS += -Wl,--no-check-sections 

# This builds a .riscv binary for the current machine type and tile
# group size. RISCV_TARGET_OBJECTS are .rvo files that will be linked
# in the final binary.
%.riscv: crt.rvo bsg_set_tile_x_y.rvo bsg_tile_config_vars.rvo main.rvo $(RISCV_TARGET_OBJECTS) $(RISCV_LINK_SCRIPT) 
	$(RISCV_LD) -T $(RISCV_LINK_SCRIPT) $(RISCV_LDFLAGS) $(filter %.rvo,$^) -o $@


%.dis: %.riscv
	$(RISCV_OBJDUMP) -d $< 2>&1 | tee $@

kernel.link.clean:
	rm -rf *.riscv $(RISCV_LINK_SCRIPT)


.PRECIOUS: %.riscv
.PHONY: kernel.link.clean kernel.compile.clean
clean: kernel.link.clean kernel.compile.clean

