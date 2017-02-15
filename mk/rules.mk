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

$(BUILD)/libchirp.a: $(LIB_OBJECTS)
ifeq ($(VERBOSE),True)
	ar $(ARFLAGS) $@ $(LIB_OBJECTS) > /dev/null 2> /dev/null
ifeq ($(STRIP),True)
	$(STRIP) $@
endif
else
	@echo AR $@
	@ar $(ARFLAGS) $@ $(LIB_OBJECTS) > /dev/null 2> /dev/null
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRIP) $@
endif
endif

$(BUILD)/libchirp.so: $(LIB_OBJECTS)
ifeq ($(VERBOSE),True)
	$(CC) -shared -o $@ $(LIB_OBJECTS) $(LDFLAGS)
ifeq ($(STRIP),True)
	$(STRIP) $@
endif
else
	@echo LD $@
	@$(CC) -shared -o $@ $(LIB_OBJECTS) $(LDFLAGS)
ifeq ($(STRIP),True)
	@echo STRIP $@
	@$(STRIP) $@
endif
endif
