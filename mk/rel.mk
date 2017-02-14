CFLAGS  := \
	-std=gnu99 \
	-fPIC \
	-Wall \
	-Wextra \
	-pedantic \
	-Wno-unused-function \
	-O3 \
	-DNDEBUG \
	-I"$(BASE)/include" \
	-I. \
	$(CFLAGS)

%.o: %.c
	@echo CC $(subst $(BASE)/,,$<)
	@mkdir -p "$(dir $(subst $(BASE)/,,$<))"
	@$(CC) -c -o "$(subst $(BASE)/,,$@)" "$<" $(CFLAGS)
