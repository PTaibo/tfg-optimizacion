GPP=g++

TARGET=test_basic.elf
CSRC=$(wildcard includes/*.c)
CPPSRC=test_basic.cpp bitmap.cpp \
		$(wildcard includes/*.cpp)
OBJ=$(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

BUILD=release
cppflags.release=-O2 -march=native -D POLYBENCH_TIME
cppflags.debug=-g3 -O0 -Wextra
CPPFLAGS=$(cppflags.$(BUILD)) -Wall
LDLIBS=-I./includes

${TARGET}: ${OBJ}
	$(GPP) ${CPPFLAGS} $^ -o $@

%.o: %.c
	$(GPP) ${CPPFLAGS} ${LDLIBS} -c $^ -o $@

%.o: %.cpp
	$(GPP) ${CPPFLAGS} ${LDLIBS} -c $^ -o $@

run: ${TARGET}
	./${TARGET}

clean:
	${RM} $(filter %.o,${OBJ})

cleanall:
	${RM} ${TARGET} $(filter %.o,${OBJ})

