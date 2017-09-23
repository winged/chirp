# Conditional flags
# =================

# Release flags (see base.mk for standard flags)
# ==============================================
CFLAGS += \
	-DNDEBUG \
	$(OPTFLAG) \
	$(GGDBFLAG) \

ifneq ($(STRIP),True)
	GGDBFLAG := -ggdb3
endif

ifeq (,$(findstring -O,$(CFLAGS)))
	OPTFLAG := -O2
endif
