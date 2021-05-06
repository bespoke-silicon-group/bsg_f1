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

ifndef __BSG_LIBRARIES_MK
__BSG_LIBRARIES_MK := 1

LIB_CSOURCES   +=
LIB_CSOURCES   += $(LIBRARIES_PATH)/bsg_manycore_config_id_to_string.c
LIB_CSOURCES   += $(LIBRARIES_PATH)/bsg_manycore_memsys.c
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_epa.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_bits.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_config.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_cuda.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_elf.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_eva.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_loader.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_memory_manager.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_origin_eva_map.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_print_int_responder.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_printing.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_request_packet_id.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_responder.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_tile.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_uart_responder.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_trace_responder.cpp
LIB_CXXSOURCES += $(LIBRARIES_PATH)/bsg_manycore_vcache.cpp

LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_bits.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_config.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_cuda.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_elf.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_eva.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_loader.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_memory_manager.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_origin_eva_map.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_printing.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_request_packet_id.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_responder.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_tile.h

LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_vcache.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_errno.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_features.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_coordinate.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_npa.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_epa.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_packet.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_response_packet.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_request_packet.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_fifo.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_memsys.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_rom.h
LIB_HEADERS += $(LIBRARIES_PATH)/bsg_manycore_regression.h

# Objects that should be compiled with strict compilation flags
LIB_STRICT_OBJECTS +=
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_responder.o
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_loader.o
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_packet_id.o
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_eva.o
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_origin_eva_map.o
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_print_int_responder.o
LIB_STRICT_OBJECTS += $(LIBRARIES_PATH)/bsg_manycore_memsys.o

# Object in the pod replication extension for CUDA
LIB_CXXSOURCES_CUDA_POD_REPL += $(LIBRARIES_PATH)/bsg_manycore_cuda_legacy_replicate.cpp

# Object in the regression library
LIB_CSOURCES_REGRESSION += $(LIBRARIES_PATH)/bsg_manycore_regression.c

# Objects that should be compiled with debug flags
LIB_DEBUG_OBJECTS  +=

LIB_OBJECTS += $(patsubst %cpp,%o,$(LIB_CXXSOURCES))
LIB_OBJECTS += $(patsubst %c,%o,$(LIB_CSOURCES))

LIB_OBJECTS_CUDA_POD_REPL += $(patsubst %cpp,%o,$(LIB_CXXSOURCES_CUDA_POD_REPL))
LIB_OBJECTS_REGRESSION    += $(patsubst %cpp,%o,$(LIB_CXXSOURCES_REGRESSION))
LIB_OBJECTS_REGRESSION    += $(patsubst %c,%o,$(LIB_CSOURCES_REGRESSION))

# I don't like these, but they'll have to do for now.
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: LDFLAGS := 
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: INCLUDES := 

include $(BSG_PLATFORM_PATH)/library.mk

$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES := -I$(LIBRARIES_PATH)
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES += -I$(LIBRARIES_PATH)/xcl
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES += -I$(LIBRARIES_PATH)/features/dma
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES += -I$(LIBRARIES_PATH)/features/profiler
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): INCLUDES += -I$(BSG_PLATFORM_PATH)

$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): CFLAGS    += -std=c11 -fPIC $(INCLUDES) -D_GNU_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE
$(LIB_OBJECTS) $(LIB_OBJECTS_CUDA_POD_REPL) $(LIB_OBJECTS_REGRESSION): CXXFLAGS  += -std=c++11 -fPIC $(INCLUDES) -D_GNU_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE
# Need to move this, eventually
#$(LIB_OBJECTS) $(PLATFORM_OBJECTS): $(BSG_MACHINE_PATH)/bsg_manycore_machine.h

$(LIB_DEBUG_OBJECTS):  CXXFLAGS += -DDEBUG
$(LIB_DEBUG_OBJECTS):  CFLAGS += -DDEBUG

$(LIB_STRICT_OBJECTS): CXXFLAGS += -Wall -Werror
$(LIB_STRICT_OBJECTS): CXXFLAGS += -Wno-unused-variable
$(LIB_STRICT_OBJECTS): CXXFLAGS += -Wno-unused-function
$(LIB_STRICT_OBJECTS): CXXFLAGS += -Wno-unused-but-set-variable

ifdef __NEWLIB
LIB_OBJECTS += $(patsubst %S,%o,$(LIB_SSOURCES))
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: LDFLAGS :=
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: $(LIB_OBJECTS)
	$(RV_AR) -rcs $@ $(LIB_OBJECTS) $(DMA_FEATURE_OBJECTS)
else
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: LD = $(CXX)
$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: $(LIB_OBJECTS)
	$(LD) -shared -Wl,-soname,$(basename $(notdir $@)) -o $@ $^ $(LDFLAGS)
endif

$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0: LDFLAGS  :=
$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0: INCLUDES :=

ifdef __NEWLIB
$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0: LDFLAGS :=
$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0: $(LIB_OBJECTS_CUDA_POD_REPL)
	$(RV_AR) -rcs $@ $(LIB_OBJECTS_CUDA_POD_REPL)
else
$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0: LD = $(CXX)
$(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0: $(LIB_OBJECTS_CUDA_POD_REPL)
	$(LD) -shared -Wl,-soname,$(basename $(notdir $@)) -o $@ $^ $(LDFLAGS)
endif

$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0: LDFLAGS  :=
$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0: INCLUDES :=

ifdef __NEWLIB
$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0: LDFLAGS :=
$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0: $(LIB_OBJECTS_REGRESSION)
	$(RV_AR) -rcs $@ $(LIB_OBJECTS_REGRESSION)
else
$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0: LD = $(CXX)
$(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0: $(LIB_OBJECTS_REGRESSION)
	$(LD) -shared -Wl,-soname,$(basename $(notdir $@)) -o $@ $^ $(LDFLAGS)
endif

.PHONY: libraries.clean
libraries.clean:
	rm -f $(LIB_OBJECTS)
	rm -f $(LIB_OBJECTS_CUDA_POD_REPL)
	rm -f $(LIB_OBJECTS_REGRESSION)
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0
	rm -f $(BSG_PLATFORM_PATH)/libbsgmc_cuda_legacy_pod_repl.so.1.0
	rm -f $(BSG_PLATFORM_PATH)/libbsg_manycore_regression.so.1.0

endif
