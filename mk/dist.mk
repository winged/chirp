PREFIX ?= /usr/local

all: libchirp.so libchirp.a

# Library
# =======
libchirp.so: libchirp.o
	$(CC) -shared -o $@ $+ $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif

libchirp.a: libchirp.o
	ar $(ARFLAGS) $@ $+
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif

# Checks
# ======
chirp_test: libchirp.so
	$(CC) -o $@ chirp_test.c -L. -lchirp $(LDFLAGS)

check: chirp_test
	@cat .keys/dh.pem | tr '%' 'a' > dh.pem
	@cat .keys/cert.pem | tr '%' 'a' > cert.pem
	LD_LIBRARY_PATH="." ./chirp_test
	@rm -f *.pem

# Install
# =======
install: all
	mkdir -p $(DEST)$(PREFIX)/lib
	cp -f libchirp.a $(DEST)$(PREFIX)/lib
	cp -f libchirp.so $(DEST)$(PREFIX)/lib/libchirp.so.$(VERSION)
	mkdir -p $(DEST)$(PREFIX)/include
	cp -f libchirp.h $(DEST)$(PREFIX)/include/libchirp.h
	cd $(DEST)$(PREFIX)/lib && ln -sf libchirp.so.$(VERSION) libchirp.so
	cd $(DEST)$(PREFIX)/lib && ln -sf libchirp.so.$(VERSION) libchirp.so.$(MAJOR)

uninstall:
	rm -f $(DEST)$(PREFIX)/lib/libchirp.so
	rm -f $(DEST)$(PREFIX)/lib/libchirp.so.$(VERSION)
	rm -f $(DEST)$(PREFIX)/lib/libchirp.so.$(MAJOR)
	rm -f $(DEST)$(PREFIX)/lib/libchirp.a
	rm -f $(DEST)$(PREFIX)/include/libchirp.h

clean:
	rm -f libchirp.o libchirp.so libchirp.a chirp_test
