DEVNULL := $(shell \
	if [ ! -e "include" ]; then \
		ln -s "$(BASE)/include"; \
	fi \
)

all: objects

testlibuv:
	@gcc -c -o "$(DTMP)/uv.o" "$(BASE)/mk/testlibuv.c" >> config.log 2>> config.log

clean:
	rm -rf "$(DTMP)"/*
	rm -rf src
