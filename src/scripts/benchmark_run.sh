#!/bin/bash

cp ./bitmap.h ./bitmap.h.bckp
# Change word_t to uint8_t
echo "8 bit word"
sed -i '/using word_t = / s/uint[0-9]\+_t/uint8_t/' ./bitmap.h > /dev/null
make cleanall > /dev/null
make cbench &> /dev/null
./benchmark.elf
# Change word_t to uint32_t
echo "32 bit word"
sed -i '/using word_t = / s/uint[0-9]\+_t/uint32_t/' ./bitmap.h > /dev/null
make cleanall > /dev/null
make cbench &> /dev/null
./benchmark.elf
# Change word_t to uint64_t
echo "64 bit word"
sed -i '/using word_t = / s/uint[0-9]\+_t/uint64_t/' ./bitmap.h > /dev/null
make cleanall > /dev/null
make cbench &> /dev/null
./benchmark.elf

mv ./bitmap.h.bckp ./bitmap.h

