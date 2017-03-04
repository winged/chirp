.PHONY += install uninstall

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
	-O2 \
	-DNDEBUG \
	-I"$(BASE)/include" \
	-I"$(BUILD)" \
	$(CFLAGS)

LDFLAGS := \
	-luv \
	-lssl \
	-lm \
	-lpthread \
	-lcrypto

include $(BASE)/mk/rules.mk

ifeq ($(DOC),True)
install: all doc
else
install: all
endif
	mkdir -p $(PREFIX)/usr/lib
	cp -f $(BUILD)/libchirp.a $(PREFIX)/usr/lib
	cp -f $(BUILD)/libchirp.so $(PREFIX)/usr/lib/libchirp.so.$(VERSION)
	mkdir -p $(PREFIX)/usr/include
	cp -f $(BASE)/include/libchirp.h $(PREFIX)/usr/include/libchirp.h
	rm -rf $(PREFIX)/usr/include/libchirp/
	cp -rf $(BASE)/include/libchirp/ $(PREFIX)/usr/include/libchirp/
	cd $(PREFIX)/usr/lib && ln -sf libchirp.so.$(VERSION) libchirp.so
	cd $(PREFIX)/usr/lib && ln -sf libchirp.so.$(VERSION) libchirp.so.$(MAJOR)
ifeq ($(DOC),True)
	rm -rf $(PREFIX)/usr/share/doc/chirp
	mkdir -p $(PREFIX)/usr/share/doc/chirp
	cp -R $(BASE)/doc/_build/html/* $(PREFIX)/usr/share/doc/chirp/
endif


uninstall:
	rm -f $(PREFIX)/usr/lib/libchirp.a
	rm -f $(PREFIX)/usr/lib/libchirp.so.$(VERSION)
	rm -f $(PREFIX)/usr/lib/libchirp.so
	rm -f $(PREFIX)/usr/lib/libchirp.so.$(MAJOR)
	rm -f $(PREFIX)/usr/include/libchirp.h
	rm -rf $(PREFIX)/usr/include/libchirp/
	rm -rf $(PREFIX)/usr/share/doc/chirp
