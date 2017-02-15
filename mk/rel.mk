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


$(BUILD)/%.o: $(BASE)/%.c
	@echo CC $<
	@mkdir -p "$(dir $@)"
	@$(CC) -c -o "$@" "$<" $(CFLAGS)

$(BUILD)/libchirp.a: $(LIB_OBJECTS)
	@echo AR $@
	@ar $(ARFLAGS) $@ $(LIB_OBJECTS) > /dev/null 2> /dev/null
	@echo STRIP $@
	@$(STRIP) $@

$(BUILD)/libchirp.so: $(LIB_OBJECTS)
	@echo LD $@
	@$(CC) -shared -o $@ $(LIB_OBJECTS) $(LDFLAGS)
	@echo STRIP $@
	@$(STRIP) $@
