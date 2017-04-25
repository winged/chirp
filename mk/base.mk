.PHONY += testlibuv testopenssl clean
DEVNULL := $(shell mkdir -p "$(DTMP)")

CFLAGS += -DCH_BUILD

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

testlibuv:
	@$(CC) -c -o "$(DTMP)/uv.o" $(CFLAGS) "$(BASE)/mk/testlibuv.c" >> config.log 2>> config.log

testopenssl:
	@$(CC) -c -o "$(DTMP)/openssl.o" $(CFLAGS) "$(BASE)/mk/testopenssl.c" >> config.log 2>> config.log

clean:
	rm -rf "$(DTMP)"/*
	rm -rf "$(BUILD)/src"
	rm -rf "$(BUILD)/include"
	rm -rf "$(BASE)/doc/_build/"*
	cd "$(BUILD)" && rm -f $(LIBRARIES)
