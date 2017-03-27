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
	-I"$(BUILD)"

LDFLAGS += \
	$(VISIBLITYFLAG) \
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
