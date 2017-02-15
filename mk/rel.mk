CFLAGS  := \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
	-pedantic \
	-Wno-unused-function \
	-O2 \
	-DNDEBUG \
	-I"$(BASE)/include" \
	-I. \
	$(CFLAGS)

include $(BASE)/mk/rules.mk


$(BUILD)/%.o: $(BASE)/%.c
	@echo CC $<
	@mkdir -p "$(dir $@)"
	@$(CC) -c -o "$@" "$<" $(CFLAGS)

libchirp.a: $(LIB_OBJECTS)
	@echo AR $@
	@ar $(ARFLAGS) $@ $(LIB_OBJECTS)
	@echo STRIP $@
	@$(STRIP) $@
