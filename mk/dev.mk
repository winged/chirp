.PHONY += test cppcheck etests todo help
.DEFAULT_GOAL := help
ALPINE_AND_CLANG := $(shell \
	[ -f /etc/apk/world ] && [ "$(CC)" == "clang" ] \
		&& echo True \
)

CFLAGS := \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-ffunction-sections \
	-fdata-sections \
	-Wno-unused-function \
	-O0 \
	-ggdb3 \
	-I"$(BASE)/include" \
	-I"$(BUILD)" \
	$(CFLAGS)

LDFLAGS := \
	-L"$(BUILD)" \
	-luv \
	-lssl \
	-lm \
	-lpthread \
	-lcrypto

ifeq ($(ALPINE_AND_CLANG),True)
test: all cppcheck todo  ## Test everything
	@echo Note: Alpine Linux clang does not support coverage
else
CFLAGS += --coverage
LDFLAGS += --coverage
test: coverage cppcheck todo
endif

help:  ## Display this help
	@cat $(MAKEFILE_LIST) | grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' | sort -k1,1 | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

etests: all
	$(BUILD)/src/chirp_etest & \
	PID=$$!; \
	sleep 1; \
	kill -2 $$PID
	$(BUILD)/src/quickcheck_etest

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
