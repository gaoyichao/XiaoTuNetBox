
BASE_DIR = $(shell pwd)
SRC_DIR = ${BASE_DIR}/src
INC_DIR = ${BASE_DIR}/include
TEST_DIR = ${BASE_DIR}/test

BUILD_DIR = ${BASE_DIR}/build
INSTALL_CFG_FILE = ${BUILD_DIR}/all

export INCLUDE_PATH += -I${INC_DIR}

SUBDIRS += src test 

${INSTALL_CFG_FILE}: build ${SUBDIRS}

src: FORCE
	@${MAKE} -C $@ OUTPUT_PATH=${BASE_DIR}/build

test: FORCE
	@${MAKE} -C $@ OUTPUT_PATH=${BASE_DIR}/build


build:
	mkdir build

FORCE:

install:
	-cp ${INC_DIR}/XiaoTuNetBox /usr/local/include -R
	-cp ${BUILD_DIR}/libXiaoTuNetBox.a /usr/local/lib

uninstall:
	-rm /usr/local/include/XiaoTuNetBox -r
	-rm /usr/local/lib/libXiaoTuNetBox.a

clean: clean_subdirs
	-rm build -r

-include ./build_tools/subdirs.mk


