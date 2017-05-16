.PHONY += install uninstall

ifneq ($(STRIP),True)
	GGDBFLAG := -ggdb3
endif

ifeq (,$(findstring -O,$(CFLAGS)))
	OPTFLAG := -O2
endif

# Only define for known compilers
ifeq ($(CC),clang)
	VISIBLITYFLAG := -fvisibility=hidden
endif
ifeq ($(CC),gcc)
	VISIBLITYFLAG := -fvisibility=hidden
endif

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
	$(OPTFLAG) \
	$(GGDBFLAG) \
	$(VISIBLITYFLAG) \
	-DNDEBUG \
	-I"$(BASE)/include" \
	-I"$(BASE)/src" \
	-I"$(BUILD)/src" \
	-I"$(BUILD)"

LDFLAGS += \
	$(VISIBLITYFLAG) \
	-luv \
	-lssl \
	-lm \
	-lpthread \
	-lcrypto

include $(BASE)/mk/rules.mk
