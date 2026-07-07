GPP=g++

CPPSRC=bitmap.cpp
OBJ=$(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

BUILD=release
cppflags.release=-O2 -march=native
cppflags.debug=-g3 -O0 -Wextra
CPPFLAGS=$(cppflags.$(BUILD))
LDLIBS=-I${PAPI_DIR}/include
LDFLAGS=-L${PAPI_DIR}/lib -lpapi

%.o: %.c
	$(CC) ${CPPFLAGS} ${LDLIBS} -c $^ -o $@

%.o: %.cpp
	$(GPP) ${CPPFLAGS} ${LDLIBS} -c $^ -o $@

cbench: bitmap.cu benchmark.cu
	nvcc -rdc=true ${LDLIBS} $^ -o benchmark.elf ${LDFLAGS}

bench:
	bash ./scripts/run_benchmarks.sh

ctest: bitmap.cu test_basic.cu
	nvcc -rdc=true $^ -o tests.elf

test: ctest
	./tests.elf

clean:
	${RM} $(filter %.o,${OBJ}) ./*.o

cleanall:
	${RM} *.elf $(filter %.o,${OBJ}) ./*.o

