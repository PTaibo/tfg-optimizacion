GCC=g++

TARGET=test_basic.elf
SRC=test_basic.cpp bitmap.cpp
OBJ=$(SRC:.cpp=.o)

BUILD=release
cppflags.release=-O2 -march=native
cppflags.debug=-g3 -O0 -Wextra
CPPFLAGS=$(cppflags.$(BUILD)) -Wall
LDLIBS=
LDFLAGS=

${TARGET}: ${OBJ}
	$(GCC) ${CFLAGS} ${LDFLAGS} $^ -o $@

%.o: %.c
	$(GCC) ${CFLAGS} ${LDLIBS} -c $^ -o $@

run: ${TARGET}
	./${TARGET}

clean:
	${RM} ${OBJ}

cleanall:
	${RM} ${TARGET} ${OBJ}

