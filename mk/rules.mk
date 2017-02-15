.PHONY: doc
UNAME_S   := $(shell uname -s)

ifeq ($(DOC),True)
doc: doc_files
else
doc:
	@echo Please reconfigure with ./configure --doc.; false
endif

ifeq ($(UNAME_S),Darwin)
	STRIPCMD := strip -S
else
	STRIPCMD := strip --strip-debug
endif

ifneq ($(UNAME_S),Darwin)
	CFLAGS += -pthread
else
	CFLAGS += -I/usr/local/opt/openssl/include
	LDFLAGS += -L/usr/local/opt/openssl/lib
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

$(BUILD)/%.rst: $(BASE)/%.c
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(BASE)/mk/c2rst $< $@
else
	@echo RST $<
	@$(BASE)/mk/c2rst $< $@

endif

$(BUILD)/%.rst: $(BASE)/%.h
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(BASE)/mk/c2rst $< $@
else
	@echo RST $<
	@$(BASE)/mk/c2rst $< $@
endif

$(BUILD)/libchirp.a: $(LIB_OBJECTS)
ifeq ($(VERBOSE),True)
	ar $(ARFLAGS) $@ $(LIB_OBJECTS) > /dev/null 2> /dev/null
ifeq ($(STRIP),True)
	$(STRIPCMD) $@
endif
else
	@echo AR $@
	@ar $(ARFLAGS) $@ $(LIB_OBJECTS) > /dev/null 2> /dev/null
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRIPCMD) $@
endif
endif

$(BUILD)/libchirp.so: $(LIB_OBJECTS)
ifeq ($(VERBOSE),True)
	$(CC) -shared -o $@ $(LIB_OBJECTS) $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRIPCMD) $@
endif
else
	@echo LD $@
	@$(CC) -shared -o $@ $(LIB_OBJECTS) $(LDFLAGS)
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRIPCMD) $@
endif
endif
