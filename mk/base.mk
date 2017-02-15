all: libraries

testlibuv:
	@gcc -c -o "$(DTMP)/uv.o" $(CFLAGS) "$(BASE)/mk/testlibuv.c" >> config.log 2>> config.log

testopenssl:
	@gcc -c -o "$(DTMP)/uv.o" $(CFLAGS) "$(BASE)/mk/testopenssl.c" >> config.log 2>> config.log

clean:
	rm -rf "$(DTMP)"/*
	rm -rf "$(BUILD)/src"
	rm -rf "$(BUILD)/include"
	rm -rf "$(BUILD)/doc/_build/*"
	cd "$(BUILD)" && rm -f $(LIBRARIES)
