#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt

def parse_file(file_name):
    data = []
    with open(file_name) as f:
        lines = f.readlines()
        bits=-1
        bitsPerBlk=-1
        wordsPerBlk=-1
        for line in lines:
            if "bit word" in line:
                val = re.search(r'\d+', line)
                bits = val.group()
            elif "bits per rank block" in line:
                val = re.search(r'\d+', line)
                bitsPerBlk = val.group()
                wordsPerBlk = -1
            elif "word per rank block" in line:
                val = re.search(r'\d+', line)
                wordsPerBlk = val.group()
                bitsPerBlk = -1
            elif "clock cycles" in line:
                val = re.search(r'\d+', line)
                cycles = val.group()
                data.append([bits, bitsPerBlk, wordsPerBlk, cycles])

    return data

def make_plot(data, bits, plot):
    bits = str(bits)

    # x_bits = 2 # Number of x values that are not word-aligned
    # x_label = ['300b', '600b', '1w', '2w', '3w', '4w', '5w', '6w', '7w', '8w', '9w', '10w', '11w', '12w', '13w', '14w', '15w', '16w', '17w', '18w', '19w', '20w', '21w', '22w', '23w', '24w', '25w', '26w', '27w', '28w', '29w', '30w', '31w', '32w', '33w', '34w', '35w', '100w', '200w']
    x_label = ['1w', '2w', '10w', '20w', '30w', '100w']
    x_coord = [i+1 for i in range(len(x_label))]
    height = []

    for line in data:
        print(line)
        if (str(line[0]) == bits):
            height.append(float(line[-1]))

    print("x_label: ")
    print(x_label)
    print("x_coord: ")
    print(x_coord)
    print("height: ")
    print(height)

    # plt.bar(x_coord, height, tick_label = x_label, width = 0.8, color = ['red', 'green'])
    plot.bar(x_coord, height)

    plot.set_xticks(x_coord)
    plot.set_xticklabels(x_label, rotation=90, ha='right')
    plot.set_xlabel('Bits per block')
    # plot.set_ylabel('Unhalted core cycles')
    plot.yaxis.set_tick_params(labelleft=True)
    plot.set_title(bits + ' bit words')

if __name__ == "__main__":
    if (len(sys.argv) <= 1):
        print("No benchmark file indicated")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()

    data = parse_file(sys.argv[1])
    _, axs = plt.subplots(1, 2, sharey=True)
    axs[0].set_ylabel('Unhalted core cycles')
    # make_plot(data, 8, axs[0])
    make_plot(data, 32, axs[0])
    make_plot(data, 64, axs[1])

    plt.suptitle(f'Benchmark: {sys.argv[1]}', fontsize=16, fontweight='bold')
    plt.show()

