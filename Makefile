

BASE_DIR = $(shell pwd)
SRC_DIR = ${BASE_DIR}/src
INC_DIR = ${BASE_DIR}/include
TEST_DIR = ${BASE_DIR}/test

BUILD_DIR = ${BASE_DIR}/build

export INCLUDE_PATH += -I${INC_DIR}

SUBDIRS += src test 

all: build ${SUBDIRS}

src: FORCE
	@${MAKE} -C $@ OUTPUT_PATH=${BASE_DIR}/build

test: FORCE
	@${MAKE} -C $@ OUTPUT_PATH=${BASE_DIR}/build


build:
	mkdir build

FORCE:

clean: clean_subdirs
	-rm build -r

-include ./build_tools/subdirs.mk


