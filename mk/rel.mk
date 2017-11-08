# Release flags (see base.mk for standard flags)
# ==============================================
CFLAGS += \
	-DNDEBUG \
	$(OPTFLAG) \
	$(GGDBFLAG) \
	-ffunction-sections \
	-fdata-sections

# Dead code elimination
# =====================
ifeq ($(UNAME_S),Darwin)
LDFLAGS += -Wl,-dead_strip
else
LDFLAGS += -Wl,--gc-sections
endif

ifneq ($(STRIP),True)
	GGDBFLAG := -ggdb3
endif

ifeq (,$(findstring -O,$(CFLAGS)))
	OPTFLAG := -O2
endif
