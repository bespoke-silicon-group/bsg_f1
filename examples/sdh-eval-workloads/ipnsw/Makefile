#####################
# Standard includes #
#####################
REPLICANT_PATH:=$(shell git rev-parse --show-toplevel)
include $(REPLICANT_PATH)/environment.mk

all:

##################
# Prepare inputs #
##################
ipnsw-eval-dir := $(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw/hb-prog-eval/ipnsw
ipnsw-inputs  = $(ipnsw-eval-dir)/data/database_music100.bin
ipnsw-inputs += $(ipnsw-eval-dir)/data/query_music100.bin
ipnsw-inputs += $(ipnsw-eval-dir)/data/music.edges.level_0
ipnsw-inputs += $(ipnsw-eval-dir)/data/music.edges.level_1
ipnsw-inputs += $(ipnsw-eval-dir)/data/music.edges.level_2
ipnsw-inputs += $(ipnsw-eval-dir)/data/music.edges.level_3

ipnsw-input := $(ipnsw-eval-dir)/data/database_music100.bin
# this rule generates all the inputs, but we just target one
# to avoid running this more than once
$(ipnsw-input):
	cd $(ipnsw-eval-dir) && bash prep.sh

#######################################
# Base clase run directory generation #
#######################################
# $1 = name
# $2 = version
# $3 = args
define run-dir
run/$1/kernel.cpp: kernel/$2/kernel.cpp
	@mkdir -p $$(dir $$@)
	@cp $$< $$@
	@echo "MAKING $$@"

run/$1/Makefile: template.mk
	@mkdir -p $$(dir $$@)
	@cat $$< > $$@
	@echo "C_ARGS += $3" >> $$@
	@echo "MAKING $$@"

.PHONY: generate-$1 build-$1 purge-$1 run-$1 profile-$1

generate-$1: run/$1/Makefile run/$1/kernel.cpp

purge-$1:
	rm -rf run/$1

build-$1: generate-$1
	+$(MAKE) -C run/$1 main.riscv

exec-$1: generate-$1
	+$(MAKE) -C run/$1 main.exec.log

profile-$1: generate-$1
	+$(MAKE) -C run/$1 main.profile.log

debug-$1: generate-$1
	+$(MAKE) -C run/$1/main.debug.log

saif-$1: generate-$1
	+$(MAKE) -C run/$1/main.saifgen.log
endef

#################################
# Common command line arguments #
#################################
C_ARGS += $(ipnsw-inputs)

###############
# Greedy Walk #
###############
# greedy-walk version -> dimensions
 #  inner product with ipc=0.3 (8x4)
greedy_walk-grp-x := 1
greedy_walk-grp-y := 1
#  inner product with ipc=0.43 (8x4)
greedy_walk_v1-grp-x := 1
greedy_walk_v1-grp-y := 1
#  inner product with FLOPS/cycle=0.2  (8x4)
greedy_walk_v2-grp-x := 1
greedy_walk_v2-grp-y := 1
#  inner product with FLOPS/cycle=0.26 (8x4)
greedy_walk_v3-grp-x := 1
greedy_walk_v3-grp-y := 1
#  inner product v4-serial
greedy_walk_v3-ipv4serial-grp-x := 1
greedy_walk_v3-ipv4serial-grp-y := 1
#  greedy_walk_v3 + ParallelInnerProduct_v1
greedy_walk_v4-grp-x := 2
greedy_walk_v4-grp-y := 2

###############
# Beam Search #
###############
# beam-search version -> dimensions

#  very slow - uses a very dumb sparse set
beam_search-grp-x := 1
beam_search-grp-y := 1
#  dense set - inner product with ipc=0.3  (8x4)
beam_search_v1-grp-x := 1
beam_search_v1-grp-y := 1
#  dense set - inner product with ipc=0.43 (8x4)
beam_search_v2-grp-x := 1
beam_search_v2-grp-y := 1
#  + inner_product_v2 (flops/cycle=0.2039) (8x4)
beam_search_v3-grp-x := 1
beam_search_v3-grp-y := 1
#  + inner_product_v3 (flops/cycle=0.2663) (8x4)
beam_search_v4-grp-x := 1
beam_search_v4-grp-y := 1
#  + Bit vector for dense set
beam_search_v5-grp-x := 1
beam_search_v5-grp-y := 1
#  + Bit vector for dense set + inner product v4 seria;
beam_search_v5-ipv4serial-grp-x := 1
beam_search_v5-ipv4serial-grp-y := 1
#  beam_search_v5 + inner_product_parallel_v3
beam_search_v6-grp-x := 2
beam_search_v6-grp-y := 2
# beam_search_v6 but with 1x2 tile group
beam_search_v7-grp-x := 1
beam_search_v7-grp-y := 2
# beam_search_v5 but edges of candidates traversed in parallel
beam_search_v8-grp-x := 4
beam_search_v8-grp-y := 4
# combination of beam_search_v8 + beam_search_v6
beam_search_v9-grp-x := 4
beam_search_v9-grp-y := 4
# beam_search_v5 but edges of candidates traversed in parallel
beam_search_v10-grp-x := 8
beam_search_v10-grp-y := 4

# $1 = version
# $2 = query
run-name = $(1)_query$(2)
define run
$(eval $(call run-dir,$(call run-name,$1,$2),$1,\
$(EXAMPLES_PATH)/sdh-eval-workloads/ipnsw/run/$(call run-name,$1,$2)/kernel.riscv \
$1 \
$(C_ARGS) \
--queries $(2) \
--group-x $($(1)-grp-x) \
--group-y $($(1)-grp-y) \
))
generate: generate-$(call run-name,$1,$2)
purge:    purge-$(call run-name,$1,$2)
build:    build-$(call run-name,$1,$2)
exec:     exec-$(call run-name,$1,$2)
profile:  profile-$(call run-name,$1,$2)
debug:    debug-$(call run-name,$1,$2)
saifgen:  saifgen-$(call run-name,$1,$2)

saifgen-$(call run-name,$1,$2): $(ipnsw-input)
profile-$(call run-name,$1,$2): $(ipnsw-input)
debug-$(call run-name,$1,$2):   $(ipnsw-input)
exec-$(call run-name,$1,$2):    $(ipnsw-input)

endef
.PHONY: generate
.PHONY: purge
.PHONY: build
.PHONY: exec
.PHONY: profile
.PHONY: debug
.PHONY: saifgen

#############################################################
# Define which queries we want to run and instantiate rules #
#############################################################
greedy-walk-queries := 4 16 229 276 461 470 490
$(foreach q,$(greedy-walk-queries),$(eval $(call run,greedy_walk_v4,$(q))))

beam-search-queries := 2   188 229 355 427 472
beam-search-queries += 25  74  112 140 148 178
beam-search-queries += 214 244 278 302 331
beam-search-queries += 396 420 452 489 511
$(foreach q,$(beam-search-queries),$(eval $(call run,beam_search_v10,$(q))))

.PHONY: all
all: exec
