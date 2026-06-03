#!/bin/bash

benchmarks=("select0"               # 0
            "select1"               # 1
            "rank"                  # 2
            "select0_vs_select1"    # 3
            "rank_vs_wrdRank")      # 4

to_run=(3)

echo "Benchmarks to run:"
for i in ${to_run[@]}; do
    echo -e "\t- ${benchmarks[$i]}"
done
echo ""

for i in ${to_run[@]}; do
# for ((i = 0; i < ${#to_run[@]}; i++)); do
    echo "Running ${benchmarks[$i]}"
    bash $BITMAP_PATH/src/scripts/single_benchmark.sh $i | tee $BITMAP_PATH/benchmarks/${benchmarks[$i]}.txt
done

