UNAME_S := $(shell uname -s)

# Standard flags
# ==============
# Will be extended depending on build-mode, system and compiler
CFLAGS += \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-Wno-unused-function \
	$(OPTFLAG) \
	$(GGDBFLAG) \
	$(VISIBLITYFLAG) \
	-ffunction-sections \
	-fdata-sections

LDFLAGS += \
	$(VISIBLITYFLAG) \
	-luv \
	-lssl \
	-lm \
	-lpthread \
	-lcrypto

# Only define for known compilers
ifeq ($(CC),clang)
	VISIBLITYFLAG := -fvisibility=hidden
endif
ifeq ($(CC),gcc)
	VISIBLITYFLAG := -fvisibility=hidden
endif
ifeq ($(CC),cc)
	_CC_OUT := $(shell cc -v 2>&1)
ifneq (,$(findstring gcc version,$(_CC_OUT)))
	VISIBLITYFLAG := -fvisibility=hidden
endif
ifneq (,$(findstring clang version,$(_CC_OUT)))
	VISIBLITYFLAG := -fvisibility=hidden
endif
endif

# Additional flags
# ================
ifeq ($(UNAME_S),Darwin)
# Homebrew include path
CFLAGS += -I/usr/local/opt/openssl/include
LDFLAGS += -L/usr/local/opt/openssl/lib
else
# Linux specific
CFLAGS += -pthread
LDFLAGS += -lrt
endif

# Strip command
# =============
ifeq ($(UNAME_S),Darwin)
STRPCMD := strip -S
else
STRPCMD:= strip --strip-debug
endif

# Dead code elimination
# =====================
ifeq ($(UNAME_S),Darwin)
LDFLAGS += -Wl,-dead_strip
else
LDFLAGS += -Wl,--gc-sections
endif

ifneq ($(STRIP),True)
	GGDBFLAG := -ggdb3
endif

ifeq (,$(findstring -O,$(CFLAGS)))
	OPTFLAG := -O0
endif

all: monitor agent collector

# Binaries
# ========
monitor: libchirp.a monitor.c common.h
	$(CC) -o $@ $@.c $< $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif

agent: libchirp.a agent.c common.h
	$(CC) -o $@ $@.c $< $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif

collector: libchirp.a collector.c common.h
	$(CC) -o $@ $@.c $< $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif

# Library
# =======
libchirp.a: libchirp.o
	ar $(ARFLAGS) $@ $+
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif

# Util
# ====
clean:
	rm -f libchirp.o libchirp.a monitor agent collector
