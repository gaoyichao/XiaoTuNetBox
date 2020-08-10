
.PHONY: clean_subdirs, subdirs 

subdirs: build
	@for subdir in ${SUBDIRS}; do \
		echo "Entering $$subdir."; \
		if ! ${MAKE} --no-print-directory -C $$subdir; then \
			echo "编译[$$subdir]失败!"; \
			exit 1; \
			fi; \
	done

clean_subdirs:
	-@for subdir in ${SUBDIRS}; do \
		echo "Entering $$subdir."; \
		${MAKE} --no-print-directory -C $$subdir clean; \
	done


