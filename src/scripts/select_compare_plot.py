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
            elif "Select0" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, 0, bitsPerBlk, wordsPerBlk, seconds])
            elif "Select1" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, 1, bitsPerBlk, wordsPerBlk, seconds])

    return data

def make_plot(data, bits, select, plot):
    bits = str(bits)
    select = str(select)

    x_label = ['1w', '2w', '10w', '20w', '30w', '100w']   
    x_coord = [i+1 for i in range(len(x_label))]
    height = []

    for line in data:
        print(line)
        if (str(line[0]) == bits and str(line[1]) == select):
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
    plot.set_xticklabels(x_label)
    plot.set_xlabel('Bits per block')
    # plot.set_ylabel('Seconds')
    plot.yaxis.set_tick_params(labelleft=True)
    plot.set_title('Select' + select + ' for ' + bits + ' bit words')

    # plotPos.plot.show()

if __name__ == "__main__":
    if (len(sys.argv) <= 1):
        print("No benchmark file indicated")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()

    data = parse_file(sys.argv[1])
    _, axs = plt.subplots(2, 2, sharey=True)
    axs[0,0].set_ylabel('Seconds')
    axs[1,0].set_ylabel('Seconds')
    make_plot(data, 32, 0, axs[0, 0])
    make_plot(data, 64, 0, axs[0, 1])
    make_plot(data, 32, 1, axs[1, 0])
    make_plot(data, 64, 1, axs[1, 1])

    plt.suptitle(f'Benchmark: {sys.argv[1]}', fontsize=16, fontweight='bold')
    plt.show()

