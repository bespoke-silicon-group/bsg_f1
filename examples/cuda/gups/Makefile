# $1 = Table size in words
# $2 = Updates per core
# $3 = Cores to use
# $4 = Concurrency
# $5 = exec|profile|debug|saifgen
test-name = $(1)_table-words__$(2)_updates-per-core__$(3)_cores__$(4)_concurrency__$(5)_run
# $1 = Test name
get-table-words = $(firstword $(subst _, ,$(filter %_table-words,$(subst __, ,$1))))
# $1 = Test name
get-updates-per-core = $(firstword $(subst _, ,$(filter %_updates-per-core,$(subst __, ,$1))))
# $1 = Test name
get-cores = $(firstword $(subst _, ,$(filter %_cores,$(subst __, ,$1))))
# $1 = Test name
get-concurrency = $(firstword $(subst _, ,$(filter %_concurrency,$(subst __, ,$1))))
# $1 = Test name
get-run-type = $(firstword $(subst _, ,$(filter %_run,$(subst __, ,$1))))

TESTS += $(call test-name,1024,16,1,1,profile)
TESTS += $(call test-name,1024,16,1,2,profile)
TESTS_DISASSEMBLY = $(addsuffix .dis,$(TESTS))

$(TESTS_DISASSEMBLY):
	$(eval t=$(firstword $(subst ., ,$@)))
	$(eval TABLE_WORDS=$(call get-table-words,$t))
	$(eval UPDATES_PER_CORE=$(call get-updates-per-core,$t))
	$(eval CORES=$(call get-cores,$t))
	$(eval RUN_TYPE=$(call get-run-type,$t))
	$(eval CONCURRENCY=$(call get-concurrency,$t))
	mkdir -p $t/
	cp template.mk $t/Makefile
	$(MAKE) -C $t EXTRA_RISCV_CCPPFLAGS="-DCONCURRENCY=$(CONCURRENCY)" \
	EXTRA_C_ARGS="$(TABLE_WORDS) $(UPDATES_PER_CORE) $(CORES)" \
	main.dis

.PHONY: $(TESTS)
$(TESTS):
	$(eval TABLE_WORDS=$(call get-table-words,$@))
	$(eval UPDATES_PER_CORE=$(call get-updates-per-core,$@))
	$(eval CORES=$(call get-cores,$@))
	$(eval RUN_TYPE=$(call get-run-type,$@))
	$(eval CONCURRENCY=$(call get-concurrency,$@))
	mkdir -p $@/
	cp template.mk $@/Makefile
	$(MAKE) -C $@ EXTRA_RISCV_CCPPFLAGS="-DCONCURRENCY=$(CONCURRENCY)" \
	EXTRA_C_ARGS="$(TABLE_WORDS) $(UPDATES_PER_CORE) $(CORES)" \
	main.$(RUN_TYPE).log

all: $(TESTS)
profile:   $(filter %__profile_run,$(TESTS))
debug:     $(filter %__debug_run,$(TESTS))
exec:      $(filter %__exec_run,$(TESTS))
saifgen:   $(filter %__saifgen_run,$(TESTS))

dis: $(TESTS_DISASSEMBLY)

purge:
	rm -rf $(TESTS)
