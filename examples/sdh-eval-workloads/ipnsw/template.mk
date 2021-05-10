REPLICANT_PATH:=$(shell git rev-parse --show-toplevel)
include $(REPLICANT_PATH)/environment.mk
include $(BSG_MACHINE_PATH)/Makefile.machine.include

# kernel code
BSG_MANYCORE_KERNELS = kernel.riscv

RISCV_CCPPFLAGS += -I$(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw/kernel/include
RISCV_CCPPFLAGS += -Dbsg_tiles_X=1
RISCV_CCPPFLAGS += -Dbsg_tiles_Y=1

RISCV_TARGET_OBJECTS = kernel.rvo
kernel.rvo: RISCV_CXX = $(RISCV_CLANGXX)
RISCV_OPT_LEVEL = -O3
include $(EXAMPLES_PATH)/cuda/riscv.mk
RISCV_LDFLAGS := $(filter-out -nostdlib,$(RISCV_LDFLAGS))

# host code
graphtools-dir := $(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw/graph-tools
hammerblade-helpers-dir := $(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw/hammerblade-helpers

include $(graphtools-dir)/libgraphtools.mk
include $(hammerblade-helpers-dir)/libhammerblade-helpers-host.mk

# header files
TEST_HEADERS := $(libhammerblade-helpers-host-interface-headers)
TEST_HEADERS += $(libgraphtools-interface-headers)
TEST_HEADERS += GreedyWalkResults.hpp
TEST_HEADERS += IO.hpp
TEST_HEADERS += IPNSWGraph.hpp
TEST_HEADERS += IPNSWRunner.hpp
TEST_HEADERS += IPNSWKernelRunner.hpp
TEST_HEADERS += GreedyWalkKernelRunner.hpp
TEST_HEADERS += BeamSearchKernelRunner.hpp
TEST_HEADERS += IProductUBmkKernelRunner.hpp
TEST_HEADERS += IPNSWResultReader.hpp
TEST_HEADERS += GreedyWalkResultReader.hpp
TEST_HEADERS += BeamSearchResultReader.hpp
TEST_HEADERS += GreedyWalkResults.hpp
TEST_HEADERS += IPNSWFactory.hpp
TEST_HEADERS += GreedyWalkFactory.hpp
TEST_HEADERS += BeamSearchFactory.hpp
TEST_HEADERS += IProductUBmkFactory.hpp
TEST_HEADERS += StringHelpers.hpp

# source files
TEST_SOURCES := GreedyWalkResults.cpp
TEST_SOURCES += ipnsw.cpp

# cxxflags
CXXFLAGS += $(libgraphtools-interface-cxxflags)
CXXFLAGS += $(libhammerblade-helpers-host-interface-cxxflags)
CXXFLAGS += -I$(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw
CXXFLAGS += -DCOSIM

# ldflags
LDFLAGS += $(libgraphtools-interface-ldflags)
LDFLAGS += $(libhammerblade-helpers-host-interface-ldflags)

vpath %.cpp $(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw
vpath %.hpp $(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw

TEST_NAME = main

include $(EXAMPLES_PATH)/compilation.mk
include $(EXAMPLES_PATH)/link.mk

# mark dependencies
$(TEST_OBJECTS): $(libgraphtools-interface-libraries)
$(TEST_OBJECTS): $(TEST_HEADERS)

include $(EXAMPLES_PATH)/execution.mk
