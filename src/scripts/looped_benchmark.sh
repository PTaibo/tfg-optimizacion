#!/bin/bash

CODE="$BITMAP_PATH/src"

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No colour

bmapSizes=(100 1000 10000 100000 1000000 10000000 100000000 1000000000)

# Command to create random bitmap file to test against
# It takes long to create, so it's recommended to run once
# before the first benchmark and reuse it
# dd if=/dev/random of=$CODE/bitmap.bin bs=1 count=125000000

cp $CODE/bitmap.h $CODE/bitmap.h.bckp
cp $CODE/bitmap.cu $CODE/bitmap.cu.bckp

for size in "${bmapSizes[@]}"; do
    echo "$size elements in bitmap"
    sed -i "s/\(size_t bmap_size = \)[0-9]\+/\1${size}/" benchmark.cu

    make -f $CODE/Makefile cleanall > /dev/null
    make -f $CODE/Makefile cbench &> /dev/null

    for i in $(seq 0 99); do
        echo "Run $i ($size)"
        $CODE/benchmark.elf $1 $2
    done
done

echo "-------------------"

mv $CODE/bitmap.h.bckp $CODE/bitmap.h
mv $CODE/bitmap.cu.bckp $CODE/bitmap.cu

