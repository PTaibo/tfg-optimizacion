# Parallelization of Compact Data Structures

This project consists on the paralellization of a bitmap compact data structure following different methods (CPU SIMD parallelization and GPU CUDA parallelization).
The bitmap was chosen for being used in many other compact data structures that could build on top of this.

### Organization of the branches of the repository
- develop: contains the basic bitmap, implemented sequentially
- cpu: contains the SIMD parallelized bitmap
- gpu: contains the CUDA parallelized bitmap

### How to run
Clone the repository and move to the branch you want to work with. Inside the `src` directory you can run the following commands.
- Compile the tests: `make ctest`
- Run the tests: `make test` (compiles before running if necessary)
- Compile the benchmarks: `make cbench`
- Run the benchmarks: `make bench` (compiles before running if necessary)
- Delete all object files: `make clean`
- Delete all object files and executables: `make cleanall`

### Benchmarks
To choose which benchmarks to run go to `src/scripts/run_benchmarks.sh` and add the wanted benchmark numbers to the to_run list (the available ones are listed inside the benchmarks list).
Running the benchmark creates a text file inside a directory `benchmark` (you might have to create before running the benchmark). To convert the meassures to visual plots you can feed the text file into the single_plot.py script, inside `src/scripts`.
For the comparison benchmarks you need to run the appropriate python script.

### Features
**Sequential implementation**:
- Returns value of specific bit
- Returns size of the bitmap in bits
- Mutable bitmap with bit-specific set, clear and toggle functions
- Can be created from a string of 0 and 1 characters, from another bitmap or initialized to zeros with a specified size. The size of the rank support stucture blocks can be manually passed as a parameter
- Allows conversion from bitmap to string of 1 and 0 characters
- Calculates select for 0s and for 1s
- Calculates word aligned ranks and non word aligned ranks (the rank support structure size does not have to be a multiple of the bitmap word size)
- Implements rank support structure for quicker rank calculations
- Lazy update algorithm for rank support structure

**CPU implementation:**
- Returns value of specific bit
- Returns size of the bitmap in bits
- Mutable bitmap with bit-specific set, clear and toggle functions
- Can be created from a string of 0 and 1 characters, from another bitmap or initialized to zeros with a specified size. The size of the rank support stucture blocks can be manually passed as a parameter
- Allows conversion from bitmap to string of 1 and 0 characters
- Calculates select for 1s
- Calculates rank using SIMD instructions. Assumes the rank support structure is always word aligned
- Implements word aligned rank support structure for quicker rank calculations
- Lazy update algorithm for rank support structure

**GPU implementation:**
- Returns value of specific bit
- Returns size of the bitmap in bits
- Immutable bitmap
- Created from a binary file
- Allows conversion from bitmap to string of 1 and 0 characters
- Calculates rank using CUDA instructions
- Does not use rank support structure

### Dependencies
- Polybench library: for the time counters in the benchmark [GitHub](https://github.com/MatthiasJReisinger/PolyBenchC-4.2.1)
- PAPI library: for the unhalted core cycles counters in the benchmark [GitHub](https://github.com/icl-utk-edu/papi)
- Thrust library: for the reduction in the CUDA implemented rank method [NVIDIA page](https://developer.nvidia.com/thrust)

