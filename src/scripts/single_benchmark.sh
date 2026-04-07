#!/bin/bash

GIT='/home/p.taibo/cuda/bitmaps/src'

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No colour

wordBits=(8 32 64)
bitsPerBlk=(3 300 600)
wordMult=(1 2 10 30)

cp $GIT/bitmap.h $GIT/bitmap.h.bckp
cp $GIT/bitmap.cpp $GIT/bitmap.cpp.bckp

for bits in "${wordBits[@]}"; do
    echo "$bits bit word"
    sed -i "/using word_t = / s/uint[0-9]\+_t/uint${bits}_t/" $GIT/bitmap.h > /dev/null

    for blk in "${bitsPerBlk[@]}"; do
        echo "$blk bits per rank block"
        sed -i "/#define RANKBLK / s/[0-9]\+/${blk}/" $GIT/bitmap.cpp

        make -f $GIT/Makefile cleanall > /dev/null
        make -f $GIT/Makefile cbench &> /dev/null
        $GIT/benchmark.elf $1
    done

    for words in "${wordMult[@]}"; do
        echo "$words word per rank block"
        sed -i "/#define RANKBLK / s/[0-9]\+/$((${words}*${bits}))/" $GIT/bitmap.cpp

        make -f $GIT/Makefile cleanall > /dev/null
        make -f $GIT/Makefile cbench &> /dev/null
        $GIT/benchmark.elf $1
    done

    echo "-------------------"
done

mv $GIT/bitmap.h.bckp $GIT/bitmap.h
mv $GIT/bitmap.cpp.bckp $GIT/bitmap.cpp

