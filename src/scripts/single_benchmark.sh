#!/bin/bash

CODE="$BITMAP_PATH/src"

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No colour

wordMult=(1 2 10 20 30 100)

cp $CODE/bitmap.h $CODE/bitmap.h.bckp
cp $CODE/bitmap.cpp $CODE/bitmap.cpp.bckp

for words in "${wordMult[@]}"; do
    echo "$words word per rank block"
    sed -i "/#define RANKBLK / s/[0-9]\+/${words}/" $CODE/bitmap.cpp

    make -f $CODE/Makefile cleanall > /dev/null
    make -f $CODE/Makefile cbench &> /dev/null
    $CODE/benchmark.elf $1
done

echo "-------------------"

mv $CODE/bitmap.h.bckp $CODE/bitmap.h
mv $CODE/bitmap.cpp.bckp $CODE/bitmap.cpp

