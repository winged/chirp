.PHONY += test cppcheck etests todo help coverage
.DEFAULT_GOAL := help

# Development flags (see base.mk for standard flags)
# ==================================================
CFLAGS += \
	-O0 \
	-ggdb3 \

LDFLAGS += -L"$(BUILD)" \

# Binary tests to run
# ===================
etests: all
	LD_LIBRARY_PATH="$(BUILD)" $(BUILD)/src/chirp_etest
	$(BUILD)/src/quickcheck_etest
	$(MEMCHECK) $(BUILD)/src/quickcheck_etest
	$(BUILD)/src/message_etest \
			2> message_etest.log || \
		(cat message_etest.log; false)
	$(MEMCHECK) $(BUILD)/src/message_etest \
			2> message_etest.log || \
		(cat message_etest.log; false)
	$(MEMCHECK) $(BUILD)/src/message_etest --always-encrypt \
			2> message_etest.log || \
		(cat message_etest.log; false)
	$(MEMCHECK) $(BUILD)/src/message_etest \
			--always-encrypt \
			--message-count 50 \
			2> message_etest.log || \
		(cat message_etest.log; false)
	$(MEMCHECK) $(BUILD)/src/message_etest \
			--always-encrypt \
			--buffer-size 1024 \
			2> message_etest.log || \
		(cat message_etest.log; false)

# Disable coverage with clang on alpine
# =====================================
# Coverage didn't work at the time of writing this code
ALPINE_AND_CLANG := $(shell \
	[ -f /etc/apk/world ] && [ "$(CC)" == "clang" ] \
		&& echo True \
)
ifeq ($(ALPINE_AND_CLANG),True)
	DISABLE_COV := True
endif

ifneq ($(TLS),openssl)
	MEMCHECK := valgrind \
		--tool=memcheck \
		--leak-check=full \
		--errors-for-leak-kinds=all \
		--show-leak-kinds=all \
		--error-exitcode=1 \
		--suppressions=$(BASE)/ci/memcheck-musl.supp
else
	MEMCHECK := valgrind \
		--tool=memcheck \
		--suppressions=$(BASE)/ci/memcheck-musl.supp
endif

# Test target
# ===========
ifeq ($(DISABLE_COV),True)
test: etests pytest cppcheck check-abi todo  ## Test everything
	@echo Note: Coverage disabled or not supported
else
CFLAGS += --coverage
LDFLAGS += --coverage
test: coverage cppcheck check-abi todo
endif

# Coverage target
# ===============
ifeq ($(DISABLE_COV),True)
coverage:
	@echo Coverage disabled
else
coverage: clean all etests pytest $(COV_FILES)  ## Analyze coverage
	!(grep -v "// NOCOV" *.gcov | grep -E "\s+#####:")
	rm -f *.gcov\n\n
endif

# Update abi target
# =================
update-abi: all  ## Update the ABI file
	cd "$(BUILD)" && abi-compliance-checker -lib chirp \
		-dump "$(BUILD)/abi-base.xml"
	cp $(BUILD)/abi_dumps/chirp/X/ABI.dump \
		$(BASE)/ci/abi_dumps/chirp/X/ABI.dump

# Check abi target
# ================
ifeq ($(CI_DISTRO),arch)
check-abi:
else
check-abi: $(BUILD)/abi_dumps/chirp/$(VERSION)/ABI.dump  ## Check the ABI
	cd "$(BUILD)" && abi-compliance-checker -lib chirp  \
		-old $(BASE)/ci/abi_dumps/chirp/X/ABI.dump \
		-new $(BUILD)/abi_dumps/chirp/$(VERSION)/ABI.dump
endif

# Rule to make abi dump
# =====================
$(BUILD)/abi_dumps/chirp/$(VERSION)/ABI.dump: libchirp.so
	cd "$(BUILD)" && abi-compliance-checker -lib chirp \
		-dump "$(BUILD)/abi-cur.xml"

# Pytest target
# =============
pytest:  ## Run pytests
	pytest $(BASE)/src
	MPP_MC=True pytest $(BASE)/src

# cppcheck target
# ===============
cppcheck: headers  ## Static analysis
	cppcheck -v \
		--enable=style,performance,portability \
		--suppress=unusedFunction \
		--suppress=*:*mpack_test.? \
		--suppress=*:*sds_test.? \
		--config-exclude="$(BASE)/src/mpack" \
		--error-exitcode=1 \
		--std=c99 \
		--inline-suppr \
		-I"$(BASE)/include" \
		-I"$(BASE)/src" \
		-I"$(BUILD)/src" \
		-DCH_ACCEPT_STRANGE_PLATFORM \
		"$(BASE)/src"

# Utility targets
# ===============
todo:  ## Show todos
	@grep -Inrs ".. todo" $(BASE)/src; true
	@grep -Inrs TODO $(BASE)/src; true

help:  ## Display this help
	@cat $(MAKEFILE_LIST) | grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' | sort -k1,1 | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
	@echo
	@echo 'Known variables: VERBOSE=True, MACRO_DEBUG=True'
