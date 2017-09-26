.PHONY += doc

# Make .o form .c files
# =====================
$(BUILD)/%.o: $(BASE)/%.c
	@mkdir -p "$(dir $@)"
ifeq ($(MACRO_DEBUG),True)
ifeq ($(VERBOSE),True)
	$(CC) $(CFLAGS) -E -P $< | clang-format > $@.c
	$(CC) -c -o $@ $@.c $(NWCFLAGS) \
			2> $@.log || \
		(cat $@.log; false)
else
	@echo MDCC $<
	@$(CC) $(CFLAGS) -E -P $< | clang-format > $@.c
	@$(CC) -c -o $@ $@.c $(NWCFLAGS) \
			2> $@.log || \
		(cat $@.log; false)
endif
else
ifeq ($(VERBOSE),True)
	$(CC) -c -o $@ $< $(CFLAGS)
else
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS)
endif
endif

# Make .h from .rg.h files
# =========================
$(BUILD)/%.h: $(BASE)/%.rg.h
	@mkdir -p "$(dir $@)"
ifeq ($(VERBOSE),True)
	$(BASE)/mk/rgc $(CC) $< $@
else
	@echo RGC $<
	@$(BASE)/mk/rgc $(CC) $< $@
endif

# Make doc (c.rst) from .c files
# ==============================
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

# Make doc (h.rst) from .h files
# ==============================
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

# Make doc (h.rg.rst) from .rg.h files
# ====================================
$(BUILD)/%.rg.h.rst: $(BASE)/%.rg.h
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

# Make lib (.a) files
# ===================
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

# Make shared objects (.so) files
# ===============================
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

# Make test binares (*_etest)
# ===========================
$(BUILD)/%_etest: $(BUILD)/%_etest.o libchirp_test.a libchirp.a
ifeq ($(VERBOSE),True)
	@if [ "$@" = "$(BUILD)/src/chirp_etest" ]; then \
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
	@if [ "$@" = "$(BUILD)/src/chirp_etest" ]; then \
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

# Make coverage files
# ===================
$(BUILD)/%.c.gcov: $(BUILD)/%.o
ifeq ($(CC),clang)
ifeq ($(UNAME_S),Darwin)
	xcrun llvm-cov gcov $<
else
	llvm-cov gcov $<
endif
else
	gcov $<
endif
