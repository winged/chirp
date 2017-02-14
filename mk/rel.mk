%.o: %.c
	@echo CC $(subst $(BASE)/,,$<)
	@mkdir -p $(dir $(subst $(BASE)/,,$<))
	@$(CC) -c -o $(subst $(BASE)/,,$@) $<
