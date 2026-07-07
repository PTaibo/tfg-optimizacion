#!/bin/bash

CODE="$BITMAP_PATH/src"

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No colour

wordBits=(32 64)
# wordMult=(1 2 3 4 5 10 15 20 25 30 35 100 200)
wordMult=(1 2 10 20 30 100)

cp $CODE/bitmap.h $CODE/bitmap.h.bckp
cp $CODE/bitmap.cu $CODE/bitmap.cu.bckp

for bits in "${wordBits[@]}"; do
    echo "$bits bit word"
    sed -i "/using word_t = / s/uint[0-9]\+_t/uint${bits}_t/" $CODE/bitmap.h > /dev/null

    for blk in "${bitsPerBlk[@]}"; do
        echo "$blk bits per rank block"
        sed -i "/#define RANKBLK / s/[0-9]\+/${blk}/" $CODE/bitmap.cu

        make -f $CODE/Makefile cleanall > /dev/null
        make -f $CODE/Makefile cbench &> /dev/null
        $CODE/benchmark.elf $1
    done

    for words in "${wordMult[@]}"; do
        echo "$words word per rank block"
        sed -i "/#define RANKBLK / s/[0-9]\+/${words}/" $CODE/bitmap.cu

        make -f $CODE/Makefile cleanall > /dev/null
        make -f $CODE/Makefile cbench &> /dev/null
        $CODE/benchmark.elf $1
    done

    echo "-------------------"
done

mv $CODE/bitmap.h.bckp $CODE/bitmap.h
mv $CODE/bitmap.cu.bckp $CODE/bitmap.cu

