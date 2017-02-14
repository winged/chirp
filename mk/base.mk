DEVNULL := $(shell \
	if [ ! -e "include" ]; then \
		ln -s "$(BASE)/include"; \
	fi \
)
