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
            elif "Select0" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, 0, bitsPerBlk, wordsPerBlk, seconds])
            elif "Select1" in line:
                val = re.search(r'\d+\.\d*', line)
                seconds = val.group()
                data.append([bits, 1, bitsPerBlk, wordsPerBlk, seconds])

    return data

def make_plot(data, bits, select):
    bits = str(bits)
    select = str(select)

    x_label = ['3b', '300b','1w', '2w', '3w', '4w']   
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
    plt.bar(x_coord, height)

    plt.xticks(x_coord, x_label)
    plt.xlabel('Bits per block')
    plt.ylabel('Seconds')
    plt.title('Select' + select + ' for ' + bits + ' bit words')

    # TODO: Change this to subplots https://matplotlib.org/stable/gallery/subplots_axes_and_figures/subplots_demo.html
    plt.show()
    # plotPos.plot.show()

if __name__ == "__main__":
    data = parse_file("bench.txt")
    # _, axs = plt.subplots(3, 3)
    make_plot(data, 8, 0)

