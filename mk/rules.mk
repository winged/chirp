UNAME_S   := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	STRIP := strip -S
else
	STRIP := strip -S
endif

ifneq ($(UNAME_S),Darwin)
	CFLAGS += -pthread
else
	CFLAGS += -I/usr/local/opt/openssl/include
	LDFLAGS += -L/usr/local/opt/openssl/lib
endif
