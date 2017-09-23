.PHONY += install uninstall testlibuv testopenssl clean

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
	-ffunction-sections \
	-fdata-sections \
	-Wno-unused-function \
	$(VISIBLITYFLAG) \
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

# Override targets
# ================
$(BUILD)/src/mpack_test.o: CFLAGS=$(NWCFLAGS)
$(BUILD)/src/rbtree.h.rst: $(BASE)/src/rbtree.h
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(BASE)/mk/c2rst $< $@
else
	@echo RST $<
	@$(BASE)/mk/c2rst $< $@
endif
$(BUILD)/src/mpipe_test.c.rst:
	@echo Skip doc for $@
$(BUILD)/src/mpack_test.h.rst:
	@echo Skip doc for $@
$(BUILD)/src/mpack_test.c.rst:
	@echo Skip doc for $@
$(BUILD)/src/mpack-config.h.rst:
	@echo Skip doc for $@

# Export API symbols only
# =======================
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

# Additional flags
# ================
ifeq ($(UNAME_S),Darwin)
# Homebrew include path
CFLAGS += -I/usr/local/opt/openssl/include
LDFLAGS += -L/usr/local/opt/openssl/lib
# Dead code
LDFLAGS += -Wl,-dead_strip
else
# Linux specific
CFLAGS += -pthread
LDFLAGS += -lrt
# Dead code
LDFLAGS += -Wl,--gc-sections
endif

# Strip command
# =============
ifeq ($(UNAME_S),Darwin)
STRPCMD := strip -S
else
STRPCMD:= strip --strip-debug
endif

# Non Werror flags for external modules
# =====================================
# We do not enforce Werror, Wall and pedantic for external modules
# NOTE: DO NOT CHANGE CFLAGS AFTER THIS OR IN rules.mk, because
# this is not a lazy assignment
NWCFLAGS:=$(filter-out -Werror,$(CFLAGS))
NWCFLAGS:=$(filter-out -Wall,$(NWCFLAGS))
NWCFLAGS:=$(filter-out -Wextra,$(NWCFLAGS))
NWCFLAGS:=$(filter-out -pedantic,$(NWCFLAGS))

# Configure check targets
# =======================
testlibuv:
	@$(CC) -c -o "testlibuv.o" \
		$(CFLAGS) "$(BASE)/mk/testlibuv.c" \
		>> config.log 2>> config.log
	@rm testlibuv.o

testopenssl:
	@$(CC) -c -o "testopenssl.o" \
		$(CFLAGS) "$(BASE)/mk/testopenssl.c"\
		>> config.log 2>> config.log
	@rm testopenssl.o

# Library files targets
# =====================
libchirp.a: $(BUILD)/libchirp.a  ## Make libchirp.a
libchirp_test.a: $(BUILD)/libchirp_test.a  ## Make libchirp_test.a
libchirp.so: $(BUILD)/libchirp.so  ## Make libchirp.so

$(BUILD)/libchirp.a: $(LIB_OBJECTS)

$(BUILD)/libchirp_test.a: $(TEST_OBJECTS)

$(BUILD)/libchirp.so: $(LIB_OBJECTS)

# Check target
# ============
check: all  ## Check basic functionality
	LD_LIBRARY_PATH="$(BUILD)" $(BUILD)/src/chirp_etest
	$(BUILD)/src/quickcheck_etest

# Doc target
# ==========
ifeq ($(DOC),True)
doc: doc_files  ## Generate documentation
	@rm -f $(BASE)/doc/inc
	@rm -f $(BASE)/doc/src
	@ln -s $(BUILD)/include $(BASE)/doc/inc
	@ln -s $(BUILD)/src $(BASE)/doc/src
	@mkdir -p $(BASE)/doc/_build/html
ifeq ($(DEV),True)
ifeq ($(VERBOSE),True)
	make -C $(BASE)/doc html 2>&1 \
		| grep -v intersphinx \
		| grep -v "ighlighting skipped" \
		| tee doc-gen.log
	@! grep -q -E "WARNING|ERROR" doc-gen.log
else # VERBOSE
	@echo DOC
	@make -C $(BASE)/doc html 2>&1 \
		| grep -v intersphinx \
		| grep -v "ighlighting skipped" \
		| tee doc-gen.log > /dev/null
	@! grep -E "WARNING|ERROR" doc-gen.log
endif
else # DEV
ifeq ($(VERBOSE),True)
	mv $(BASE)/doc/development.rst $(BASE)/doc/development.dis;  \
	make -C $(BASE)/doc html; \
	mv $(BASE)/doc/development.dis $(BASE)/doc/development.rst
else  # VERBOSE
	@echo DOC
	@mv $(BASE)/doc/development.rst $(BASE)/doc/development.dis;  \
	make -C $(BASE)/doc html; \
	mv $(BASE)/doc/development.dis $(BASE)/doc/development.rst
endif
endif
else # DOC
doc:
	@echo Please reconfigure with ./configure --doc.; false
endif

# Install target
# ==============
ifeq ($(DOC),True)
install: all doc
else
install: all  ## Install chirp
endif
	mkdir -p $(DEST)$(PREFIX)/lib
	cp -f $(BUILD)/libchirp.a $(DEST)$(PREFIX)/lib
	cp -f $(BUILD)/libchirp.so $(DEST)$(PREFIX)/lib/libchirp.so.$(VERSION)
	mkdir -p $(DEST)$(PREFIX)/include
	cp -f $(BASE)/include/libchirp.h $(DEST)$(PREFIX)/include/libchirp.h
	rm -rf $(DEST)$(PREFIX)/include/libchirp/
	cp -rf $(BASE)/include/libchirp/ $(DEST)$(PREFIX)/include/libchirp/
	cd $(DEST)$(PREFIX)/lib && ln -sf libchirp.so.$(VERSION) libchirp.so
	cd $(DEST)$(PREFIX)/lib && ln -sf libchirp.so.$(VERSION) libchirp.so.$(MAJOR)
ifeq ($(DOC),True)
	rm -rf $(DEST)$(PREFIX)/share/doc/chirp
	mkdir -p $(DEST)$(PREFIX)/share/doc/chirp
	cp -R $(BASE)/doc/_build/html/* $(DEST)$(PREFIX)/share/doc/chirp/
endif

# Utility targets
# ===============
uninstall:  ## Uninstall chirp
	rm -f $(DEST)$(PREFIX)/lib/libchirp.a
	rm -f $(DEST)$(PREFIX)/lib/libchirp.so.$(VERSION)
	rm -f $(DEST)$(PREFIX)/lib/libchirp.so
	rm -f $(DEST)$(PREFIX)/lib/libchirp.so.$(MAJOR)
	rm -f $(DEST)$(PREFIX)/include/libchirp.h
	rm -rf $(DEST)$(PREFIX)/include/libchirp/
	rm -rf $(DEST)$(PREFIX)/share/doc/chirp

clean:  # Clean chirp
ifeq ($(VERBOSE),True)
	cd "$(BUILD)" && find . \
		! -name 'Makefile' \
		! -name '.keep' \
		! -name 'abi-*.xml' \
		! -name 'pfix' \
		! -name '*.pem' \
		! -name 'config.h' \
		! -name 'config.log' \
		-type f -exec rm -f {} +
	cd "$(BUILD)" && rm -rf */
else
	@echo Clean
	@cd "$(BUILD)" && find . \
		! -name 'Makefile' \
		! -name '.keep' \
		! -name 'abi-*.xml' \
		! -name 'pfix' \
		! -name '*.pem' \
		! -name 'config.h' \
		! -name 'config.log' \
		-type f -exec rm -f {} +
	@cd "$(BUILD)" && rm -rf */
endif
