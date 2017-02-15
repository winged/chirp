.PHONY: install uninstall

CFLAGS := \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
	-pedantic \
	-ffunction-sections \
	-fdata-sections \
	-Wno-unused-function \
	-O2 \
	-DNDEBUG \
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

install: all
	mkdir -p $(PREFIX)/usr/lib
	cp -f $(BUILD)/libchirp.a $(PREFIX)/usr/lib
	cp -f $(BUILD)/libchirp.so $(PREFIX)/usr/lib/libchirp.so.$(VERSION)
	cd $(PREFIX)/usr/lib && ln -sf libchirp.so.$(VERSION) libchirp.so
	cd $(PREFIX)/usr/lib && ln -sf libchirp.so.$(VERSION) libchirp.so.$(MAJOR)

uninstall:
	rm -f $(PREFIX)/usr/lib/libchirp.a
	rm -f $(PREFIX)/usr/lib/libchirp.so.$(VERSION)
	rm -f $(PREFIX)/usr/lib/libchirp.so
	rm -f $(PREFIX)/usr/lib/libchirp.so.$(MAJOR)
