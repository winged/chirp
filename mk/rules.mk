.PHONY += doc
UNAME_S := $(shell uname -s)

libchirp.a: $(BUILD)/libchirp.a
libchirp_test.a: $(BUILD)/libchirp_test.a
libchirp.so: $(BUILD)/libchirp.so

$(BUILD)/libchirp.a: $(LIB_OBJECTS)

$(BUILD)/libchirp_test.a: $(TEST_OBJECTS)

$(BUILD)/libchirp.so: $(LIB_OBJECTS)

check: all
	LD_LIBRARY_PATH="$(BUILD)" $(BUILD)/src/chirp_etest
	$(BUILD)/src/quickcheck_etest
	$(BUILD)/src/buffer_etest

ifeq ($(DOC),True)
doc: doc_files
	@rm -f $(BASE)/doc/inc
	@rm -f $(BASE)/doc/src
	@ln -s $(BUILD)/include $(BASE)/doc/inc
	@ln -s $(BUILD)/src $(BASE)/doc/src
	@mkdir -p $(BASE)/doc/_build/html
	@cp -f $(BASE)/doc/sglib-1.0.4/doc/index.html \
		$(BASE)/doc/_build/html/sglib.html
ifeq ($(VERBOSE),True)
	make -C $(BASE)/doc html 2>&1 | tee $(DTMP)/doc.out
	@! grep -q -E "WARNING|ERROR" $(DTMP)/doc.out
else
	@echo DOC
	@make -C $(BASE)/doc html 2>&1 | tee $(DTMP)/doc.out > /dev/null
	@! grep -E "WARNING|ERROR" $(DTMP)/doc.out
endif
else
doc:
	@echo Please reconfigure with ./configure --doc.; false
endif

ifeq ($(UNAME_S),Darwin)
STRPCMD := strip -S
else
STRPCMD:= strip --strip-debug
endif

ifeq ($(UNAME_S),Darwin)
CFLAGS += -I/usr/local/opt/openssl/include
LDFLAGS += -Wl,-dead_strip
LDFLAGS += -L/usr/local/opt/openssl/lib
else
CFLAGS += -pthread
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -lrt
endif

$(BUILD)/%.o: $(BASE)/%.c
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(CC) -c -o "$@" "$<" $(CFLAGS)
else
	@echo CC $<
	@$(CC) -c -o "$@" "$<" $(CFLAGS)
endif

$(BUILD)/%.c.rst: $(BASE)/%.c
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(BASE)/mk/twsp $<
	$(BASE)/mk/c2rst $< $@
else
	@echo TWSP $<
	@$(BASE)/mk/twsp $<
	@echo RST $<
	@$(BASE)/mk/c2rst $< $@
endif

$(BUILD)/%.h.rst: $(BASE)/%.h
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(BASE)/mk/twsp $<
	$(BASE)/mk/c2rst $< $@
else
	@echo TWSP $<
	@$(BASE)/mk/twsp $<
	@echo RST $<
	@$(BASE)/mk/c2rst $< $@
endif

$(BUILD)/%.a:
ifeq ($(VERBOSE),True)
	ar $(ARFLAGS) $@ $+
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif
else
	@echo AR $@
	@ar $(ARFLAGS) $@ $+ > /dev/null 2> /dev/null
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRPCMD) $@
endif
endif

$(BUILD)/%.so:
ifeq ($(VERBOSE),True)
	$(CC) -shared -o $@ $+ $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif
else
	@echo LD $@
	@$(CC) -shared -o $@ $+ $(LDFLAGS)
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRPCMD) $@
endif
endif

$(BUILD)/%_etest: $(BUILD)/%_etest.o libchirp.a
ifeq ($(VERBOSE),True)
	@if [ "$@" == "$(BUILD)/src/chirp_etest" ]; then \
		LIBCHIRP="-L$(BUILD) -lchirp"; \
	else \
		LIBCHIRP="$(BUILD)/libchirp.a"; \
	fi; \
	echo $(CC) -o $@ $< $(BUILD)/libchirp_test.a $$LIBCHIRP $(LDFLAGS); \
	$(CC) -o $@ $< $(BUILD)/libchirp_test.a $$LIBCHIRP $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRPCMD) $@
endif
else
	@echo LD $@
	@if [ "$@" == "$(BUILD)/src/chirp_etest" ]; then \
		LIBCHIRP="-L$(BUILD) -lchirp"; \
	else \
		LIBCHIRP="$(BUILD)/libchirp.a"; \
	fi; \
	$(CC) -o $@ $< $(BUILD)/libchirp_test.a $$LIBCHIRP $(LDFLAGS)
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRPCMD) $@
endif
endif
