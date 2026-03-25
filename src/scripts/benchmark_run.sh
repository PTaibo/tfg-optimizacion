#!/bin/bash

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No colour

wordBits=(8 32 64)
bitsPerBlk=(3 300 600)
wordMult=(1 2 10 30)

cp ./bitmap.h ./bitmap.h.bckp
cp ./bitmap.cpp ./bitmap.cpp.bckp

for bits in "${wordBits[@]}"; do
    echo "$bits bit word"
    sed -i "/using word_t = / s/uint[0-9]\+_t/uint${bits}_t/" ./bitmap.h > /dev/null

    for blk in "${bitsPerBlk[@]}"; do
        echo "$blk bits per rank block"
        sed -i "/#define RANKBLK / s/[0-9]\+/${blk}/" ./bitmap.cpp

        make cleanall > /dev/null
        make cbench &> /dev/null
        ./benchmark.elf
    done

    for words in "${wordMult[@]}"; do
        echo "$words word per rank block"
        sed -i "/#define RANKBLK / s/[0-9]\+/$((${words}*${bits}))/" ./bitmap.cpp

        make cleanall > /dev/null
        make cbench &> /dev/null
        ./benchmark.elf
    done

    echo "-------------------"
done

mv ./bitmap.h.bckp ./bitmap.h
mv ./bitmap.cpp.bckp ./bitmap.cpp

