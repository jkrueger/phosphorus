all: $(all_targets)

clean:
	$(VERBOSE)rm -fr $(all_targets)
	$(VERBOSE)rm -fr $(all_headers)
	$(VERBOSE)rm -fr $(all_objects)
	$(VERBOSE)rm -fr $(all_deps)
	@if [ -n "`find $(BUILD_DIR) -type f`" ]; then \
		echo Incomplete dependencies detected. The reliability of the \
		system can not be ; \
		echo guaranteed. Untracked objects:;\
		find $(BUILD_DIR) -type f; \
	fi;

.PHONY: all clean
