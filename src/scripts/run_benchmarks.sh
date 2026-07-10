#!/bin/bash

benchmarks=("select1"               # 0
            "rank")                 # 1

to_run=(1)

echo "Benchmarks to run:"
for i in ${to_run[@]}; do
    echo -e "\t- ${benchmarks[$i]}"
done
echo ""

SEED=15420
echo -e "Seed is ${SEED}"
echo ""

for i in ${to_run[@]}; do
# for ((i = 0; i < ${#to_run[@]}; i++)); do
    echo "Running ${benchmarks[$i]}"

    # bash $BITMAP_PATH/src/scripts/looped_benchmark_blkSize.sh $i $SEED | tee $BITMAP_PATH/benchmarks/${benchmarks[$i]}_simd.txt

    bash $BITMAP_PATH/src/scripts/looped_benchmark_bmapSize.sh $i $SEED | tee $BITMAP_PATH/benchmarks/${benchmarks[$i]}_simd_bmapSize.txt
done

