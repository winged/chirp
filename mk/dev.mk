.PHONY += test cppcheck etests todo help
.DEFAULT_GOAL := help
ALPINE_AND_CLANG := $(shell \
	[ -f /etc/apk/world ] && [ "$(CC)" == "clang" ] \
		&& echo True \
)

CFLAGS += \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-ffunction-sections \
	-fdata-sections \
	-Wno-unused-function \
	$(VISIBLITYFLAG) \
	-O0 \
	-ggdb3 \
	-I"$(BASE)/include" \
	-I"$(BUILD)"

LDFLAGS += \
	$(VISIBLITYFLAG) \
	-L"$(BUILD)" \
	-luv \
	-lssl \
	-lm \
	-lpthread \
	-lcrypto

ifeq ($(ALPINE_AND_CLANG),True)
test: all cppcheck check-abi todo  ## Test everything
	@echo Note: Alpine Linux clang does not support coverage
else
CFLAGS += --coverage
LDFLAGS += --coverage
test: coverage cppcheck check-abi todo
endif

help:  ## Display this help
	@cat $(MAKEFILE_LIST) | grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' | sort -k1,1 | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

update-abi: all  ## Update the ABI file
	cd "$(BASE)/build" && abi-compliance-checker -lib chirp \
		-dump "$(BUILD)/abi-base.xml"

check-abi: $(BASE)/build/abi_dumps/chirp/chirp_$(VERSION).abi.tar.gz  ## Check the ABI

$(BASE)/build/abi_dumps/chirp/chirp_$(VERSION).abi.tar.gz: libchirp.so
	cd "$(BASE)/build" && abi-compliance-checker -lib chirp \
		-dump "$(BUILD)/abi-cur.xml"
	cd "$(BASE)/build" && abi-compliance-checker -lib chirp  \
		-old abi_dumps/chirp/chirp_0.0.0.abi.tar.gz \
		-new abi_dumps/chirp/chirp_$(VERSION).abi.tar.gz

etests: all
	LD_LIBRARY_PATH="$(BUILD)" $(BUILD)/src/chirp_etest
	$(BUILD)/src/quickcheck_etest
	$(BUILD)/src/buffer_etest
	valgrind --suppressions=$(BASE)/ci/memcheck-musl.supp \
		$(BUILD)/src/buffer_etest

cppcheck:  ## Static analysis
	cppcheck -v \
		--error-exitcode=1 \
		--std=c99 \
		--inline-suppr \
		-I"$(BASE)/include" \
		-DCH_ACCEPT_STRANGE_PLATFORM \
		"$(BASE)/src"

todo:  ## Show todos
	@grep -Inrs ".. todo" $(BASE)/src; true
	@grep -Inrs TODO $(BASE)/src; true

include $(BASE)/mk/rules.mk

$(BUILD)/%.c.gcov: $(BUILD)/%.o
ifeq ($(CC),clang)
ifeq ($(UNAME_S),Darwin)
	xcrun llvm-cov gcov $<
else
	llvm-cov gcov $<
endif
else
	gcov $<
endif
ifneq ($(IGNORE_COV),True)
	[ -f "$(notdir $@)" ]
endif
