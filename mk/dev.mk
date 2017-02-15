CFLAGS := \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
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
	-Wl,--gc-sections \
	-luv \
	-lssl \
	-lm \
	-lpthread \
	-lcrypto

include $(BASE)/mk/rules.mk
