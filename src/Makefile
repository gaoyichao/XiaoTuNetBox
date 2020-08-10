
INCLUDE_PATH += -I.

${OUTPUT_PATH}/libXiaoTuNetBox.a: objs
	ar crv $@ ${OBJ}

-include ../build_tools/obj.mk

clean:
	@find ${DIR_PRO} -type f \
		\( -name '*.o' -o -name '*.exe' -o -name '*~' \
		-o -name '*.swo' -o -name '*.swp' -o -name '*.a' \) -print \
		| xargs rm -f


