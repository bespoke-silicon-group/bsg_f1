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

ifndef __BSG_PLATFORM_MK
__BSG_PLATFORM_MK := 1

ORANGE=\033[0;33m
RED=\033[0;31m
NC=\033[0m

# BSG_F1_DIR: The path to the BSG F1 repository
ifndef BSG_F1_DIR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: BSG_F1_DIR is not defined$(NC)"))
endif

# BSG_PLATFORM defines the platform to run or simulate on while
# running examples/regression. Current options are aws-vcs and
# dpi-verilator for simulation, and aws-fpga for emulation.

# We default to simulating the AWS machine uinsg Synopsys VCS-MX,
# HOWEVER, if VCS_HOME is not defined then we will assume that
# VCS/VCS-MX is not installed and try fall-back options
BSG_PLATFORM ?= bigblade-vcs

# FIRST check if BSG_PLATFORM is valid. It should match of the
# directories in libraries/platforms
PLATFORMS_PATH := $(BSG_F1_DIR)/libraries/platforms
AVAILABLE_PLATFORMS := $(subst $(PLATFORMS_PATH)/,,$(wildcard $(PLATFORMS_PATH)/*) $(PLATFORMS_PATH)/)
ifeq ($(filter $(BSG_PLATFORM),$(AVAILABLE_PLATFORMS)),)
    $(error $(shell echo -e "$(RED)BSG MAKE ERROR: BSG_PLATFORM value $(BSG_PLATFORM) is not a valid platform$(NC)"))
endif

# If aws-vcs is specified and VCS is installed, continue as normal
ifeq ($(BSG_PLATFORM),aws-vcs)
ifdef VCS_HOME
# Continue as normal with aws-vcs

else ifdef AGFI
# If VCS is not detected, check for AWS.
# We define AGFI in the environment of every AMI we bulid, so this should be defined.
BSG_PLATFORM := aws-fpga
else
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: Platform aws-vcs specified, but VCS_HOME not defined by environment"))
endif
endif


# If verilator is specified, check for VERILATOR_ROOT and VERILATOR
ifeq ($(BSG_PLATFORM),dpi-verilator)
ifndef VERILATOR_ROOT
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: VERILATOR_ROOT not defined$(NC)"))
endif

ifndef VERILATOR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: VERILATOR not defined$(NC)"))
endif
endif

# If using Dromajo, check if BLACKPARROT_SDK_DIR is defined
ifeq ($(BSG_PLATFORM),dromajo-vcs)
ifndef BLACKPARROT_SDK_DIR
$(error $(shell echo -e "$(RED)BSG MAKE ERROR: BLACKPARROT_SDK_DIR not defined$(NC)"))
endif
endif

# BSG Platform Path is the path to the target platform, i.e. the
# substrate that actually runs the machine. 

# To switch platforms, simply switch the path of BSG_PLATFORM_PATH to
# another directory with a platform.mk file.
BSG_PLATFORM_PATH ?= $(PLATFORMS_PATH)/$(BSG_PLATFORM)

# Convert the machine path to an abspath
override BSG_PLATFORM_PATH := $(abspath $(BSG_PLATFORM_PATH))

undefine PLATFORMS_PATH
undefine AVAILABLE_PLATFORMS

endif
