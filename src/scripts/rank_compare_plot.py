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
        wordsPerBlk=-1
        for line in lines:
            if "bit word" in line:
                val = re.search(r'\d+', line)
                bits = val.group()
            elif "word per rank block" in line:
                val = re.search(r'\d+', line)
                wordsPerBlk = val.group()
            elif "wrdRank" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, 1, wordsPerBlk, seconds])
            elif "Rank" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, 0, wordsPerBlk, seconds])

    return data

def make_plot(data, bits, plot):
    bits = str(bits)

    x_label = ['1w', '2w', '3w', '4w', '5w', '6w', '7w', '8w', '9w', '10w', '11w', '12w', '13w', '14w', '15w', '16w', '17w', '18w', '19w', '20w', '21w', '22w', '23w', '24w', '25w', '26w', '27w', '28w', '29w', '30w', '31w', '32w', '33w', '34w', '35w', '100w', '200w']
    x_coord = [i+1 for i in range(len(x_label))]
    height_rank = []
    height_word = []

    for line in data:
        if (str(line[0]) == bits):
            if (str(line[1]) == '0'):
                height_rank.append(float(line[-1]))
            elif (str(line[1]) == '1'):
                height_word.append(float(line[-1]))

    print("x_label: ")
    print(x_label)
    print("x_coord: ")
    print(x_coord)
    print("height rank: ")
    print(height_rank)
    print("height word: ")
    print(height_word)


    # plt.bar(x_coord, height, tick_label = x_label, width = 0.8, color = ['red', 'green'])
    plot.bar(x_coord, height_rank, width=0.8, color = 'blue', label='Rank', alpha=0.7)
    plot.bar(x_coord, height_word, width=0.8, color = 'yellow', label='wrdRank', alpha=0.7)

    plot.set_xticks(x_coord)
    plot.set_xticklabels(x_label, rotation=90, ha='center')
    plot.set_xlabel('Words per block')
    # plot.set_ylabel('Seconds')
    plot.legend()
    plot.yaxis.set_tick_params(labelleft=True)
    plot.grid(axis='y', linestyle='--', alpha=0.3)

    # plotPos.plot.show()

if __name__ == "__main__":
    if (len(sys.argv) <= 1):
        print("No benchmark file indicated")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()

    data = parse_file(sys.argv[1])
    _, axs = plt.subplots(2, 1, sharey=True)
    axs[0].set_ylabel('Seconds')
    axs[1].set_ylabel('Seconds')
    # make_plot(data, 8, 0, axs[0, 0])
    make_plot(data, 32, axs[0])
    make_plot(data, 64, axs[1])
    # make_plot(data, 8, 1, axs[1, 0])
    # make_plot(data, 32, 1, axs[1, 0])
    # make_plot(data, 64, 1, axs[1, 1])

    plt.suptitle(f'Benchmark: {sys.argv[1]}', fontsize=16, fontweight='bold')
    plt.show()

