CC=gcc
GPP=g++

# CSRC=$(wildcard polybench-includes/*.c)
CPPSRC=bitmap.cpp
OBJ=$(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

BUILD=release
# cppflags.release=-O2 -march=native -D POLYBENCH_PAPI
cppflags.release=-O2 -march=native
cppflags.debug=-g3 -O0 -Wextra
CPPFLAGS=$(cppflags.$(BUILD))
# LDLIBS=-I./polybench-includes -I${PAPI_DIR}/include
LDLIBS=-I${PAPI_DIR}/include
LDFLAGS=-L${PAPI_DIR}/lib -lpapi

%.o: %.c
	$(CC) ${CPPFLAGS} ${LDLIBS} -c $^ -o $@

%.o: %.cpp
	$(GPP) ${CPPFLAGS} ${LDLIBS} -c $^ -o $@

cbench: benchmark.o ${OBJ}
	$(GPP) ${CPPFLAGS} ${LDFLAGS} $^ -o benchmark.elf

bench:
	bash ./scripts/run_benchmarks.sh

ctest: test_basic.o ${OBJ}
	$(GPP) ${CPPFLAGS} $^ -o tests.elf

test: ctest
	./tests.elf

clean:
	${RM} $(filter %.o,${OBJ}) ./*.o

cleanall:
	${RM} *.elf $(filter %.o,${OBJ}) ./*.o

