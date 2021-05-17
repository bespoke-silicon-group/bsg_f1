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

# aws-fpga does not provide a DMA feature. Therefore, we compile
# features/dma/noimpl/bsg_manycore_dma.cpp that simply returns
# HB_MC_NO_IMPL for each function call.
DMA_FEATURE_CXXSOURCES += $(LIBRARIES_PATH)/features/dma/noimpl/bsg_manycore_dma.cpp

DMA_FEATURE_OBJECTS += $(patsubst %cpp,%o,$(DMA_FEATURE_CXXSOURCES))
DMA_FEATURE_OBJECTS += $(patsubst %c,%o,$(DMA_FEATURE_CSOURCES))

$(DMA_FEATURE_OBJECTS): INCLUDES := -I$(LIBRARIES_PATH)
$(DMA_FEATURE_OBJECTS): INCLUDES += -I$(LIBRARIES_PATH)/features/dma
$(DMA_FEATURE_OBJECTS): CFLAGS   := -std=c11 -fPIC -D_GNU_SOURCE -D_DEFAULT_SOURCE $(INCLUDES)
$(DMA_FEATURE_OBJECTS): CXXFLAGS := -std=c++11 -fPIC -D_GNU_SOURCE -D_DEFAULT_SOURCE $(INCLUDES)

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.so.1.0: $(DMA_FEATURE_OBJECTS)

$(BSG_PLATFORM_PATH)/libbsg_manycore_runtime.a: $(DMA_FEATURE_OBJECTS)

.PHONY: dma_feature.clean
dma_feature.clean:
	rm -f $(DMA_FEATURE_OBJECTS)

platform.clean: dma_feature.clean
