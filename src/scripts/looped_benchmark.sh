#!/bin/bash

CODE="$BITMAP_PATH/src"

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No colour

wordMult=(1 2 10 20 30 100)

# Command to create random bitmap file to test against
# It takes long to create, so it's recommended to run once
# before the first benchmark and reuse it
# dd if=/dev/random of=$CODE/bitmap.bin bs=1 count=125000000

cp $CODE/bitmap.h $CODE/bitmap.h.bckp
cp $CODE/bitmap.cu $CODE/bitmap.cu.bckp

for words in "${wordMult[@]}"; do
    echo "$words word per rank block"
    sed -i "/#define RANKBLK / s/[0-9]\+/${words}/" $CODE/bitmap.cu

    make -f $CODE/Makefile cleanall > /dev/null
    make -f $CODE/Makefile cbench &> /dev/null

    for i in $(seq 0 99); do
        echo "Run $i ($words)"
        $CODE/benchmark.elf $1 $2
    done
done

echo "-------------------"

mv $CODE/bitmap.h.bckp $CODE/bitmap.h
mv $CODE/bitmap.cu.bckp $CODE/bitmap.cu

