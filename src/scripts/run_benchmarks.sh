#!/bin/bash

GIT='/home/p.taibo/cuda/bitmaps'

benchmarks=("select1"               # 0
            "rank")                 # 1

to_run=(1)

echo "Benchmarks to run:"
for i in ${to_run[@]}; do
    echo -e "\t- ${benchmarks[$i]}"
done
echo ""

for i in ${to_run[@]}; do
# for ((i = 0; i < ${#to_run[@]}; i++)); do
    echo "Running ${benchmarks[$i]}"
    bash $GIT/src/scripts/single_benchmark.sh $i | tee $GIT/benchmarks/${benchmarks[$i]}.txt
done

