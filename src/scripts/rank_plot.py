import re
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
            elif "Rank" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, bitsPerBlk, wordsPerBlk, seconds])

    return data

def make_plot(data, bits, plot):
    bits = str(bits)

    x_label = ['3b', '300b', '600b', '1w', '2w', '10w', '30w']   
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
    plot.bar(x_coord, height, color = ['blue', 'blue', 'blue', 'green', 'green', 'green', 'green'])

    plot.set_xticks(x_coord)
    plot.set_xticklabels(x_label)
    plot.set_xlabel('Bits per block')
    plot.set_ylabel('Seconds')
    plot.set_title(bits + ' bit words')

if __name__ == "__main__":
    data = parse_file("bench.txt")
    _, axs = plt.subplots(1, 3)
    make_plot(data, 8, axs[0])
    make_plot(data, 32, axs[1])
    make_plot(data, 64, axs[2])
    plt.show()

