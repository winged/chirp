all:
	$(MAKE) -C build doc_files all 2>&1 | build/pfix
