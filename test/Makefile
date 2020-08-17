
INCLUDE_PATH += -I.

LIB_PATH += -L../build
LD_FLAGS += ${LIB_PATH} -lXiaoTuNetBox -lgtest -lgtest_main -lpthread -lrt

LIB_FILES += ${OUTPUT_PATH}/libBasicData.a


C_FILE = $(wildcard *.c)
EXE = $(patsubst %.c, %.exe,$(notdir ${C_FILE}))

CPP_FILE = $(wildcard *.cpp)
EXE = $(patsubst %.cpp, %.exe,$(notdir ${CPP_FILE}))

TARGET += ${EXE}

all: objs ${TARGET}
	mv *.exe ${OUTPUT_PATH}

%.exe:%.o
	g++ $< ${LD_FLAGS} -o $@

-include ../build_tools/obj.mk

clean:
	@find ${DIR_PRO} -type f \
		\( -name '*.o' -o -name '*.exe' -o -name '*~' \
		-o -name '*.swo' -o -name '*.swp' -o -name '*.a' \) -print \
		| xargs rm -f

