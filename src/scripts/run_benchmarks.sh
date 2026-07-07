#!/bin/bash

benchmarks=("rank")                 # 0

to_run=(0)

echo "Benchmarks to run:"
for i in ${to_run[@]}; do
    echo -e "\t- ${benchmarks[$i]}"
done
echo ""

SEED=15420
echo -e "La semilla es ${SEED}"
echo ""

for i in ${to_run[@]}; do
# for ((i = 0; i < ${#to_run[@]}; i++)); do
    echo "Running ${benchmarks[$i]}"
    bash $BITMAP_PATH/src/scripts/looped_benchmark.sh $i $SEED | tee $BITMAP_PATH/benchmarks/${benchmarks[$i]}_cuda.txt
done

