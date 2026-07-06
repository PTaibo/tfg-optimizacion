#!/bin/bash

benchmarks=("select0"               # 0
            "select1"               # 1
            "rank"                  # 2
            "select0_vs_select1"    # 3
            "rank_vs_wrdRank")      # 4

to_run=(1)

echo "Benchmarks to run:"
for i in ${to_run[@]}; do
    echo -e "\t- ${benchmarks[$i]}"
done

#SEED=$RANDOM
SEED=15420
echo -e "La semilla es ${SEED}"
echo ""

for i in ${to_run[@]}; do
# for ((i = 0; i < ${#to_run[@]}; i++)); do
    echo "Running ${benchmarks[$i]}"
    # TRY DIFFERENT WORD SIZES
    # bash $BITMAP_PATH/src/scripts/single_benchmark.sh $i $SEED | tee $BITMAP_PATH/benchmarks/${benchmarks[$i]}.txt
    # BENCH ONE WORD SIZE
    bash $BITMAP_PATH/src/scripts/looped_benchmark.sh $i $SEED | tee $BITMAP_PATH/benchmarks/${benchmarks[$i]}_looped.txt
done

