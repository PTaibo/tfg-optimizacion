#!/bin/bash

GIT='/home/p.taibo/cuda/bitmaps'

benchmarks=("select0"               # 0
            "select1"               # 1
            "rank"                  # 2
            "select0_vs_select1"    # 3
            "rank_vs_wrdRank")      # 4

for ((i = 0; i < ${#benchmarks[@]}; i++)); do
    echo "Running ${benchmarks[$i]}"
    bash $GIT/src/scripts/single_benchmark.sh $i | tee $GIT/benchmarks/${benchmarks[$i]}.txt
done

