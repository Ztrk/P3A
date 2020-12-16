import matplotlib.pyplot as plt
from math import sqrt

def mean(x):
    return sum(x) / len(x)

def sd(x):
    avg = mean(x)
    square_sum = sum([(xi - avg)**2 for xi in x])
    return sqrt(square_sum / (len(x) - 1))

def plot_mwft_time(times, mwft):
    fig, ax = plt.subplots()
    ax.plot(times, mwft)
    ax.set_xlabel('Czas[s]')
    ax.set_ylabel('Średnie znormalizowane MWFT')
    ax.set_title('MWFT w zależności od czasu')

    plt.savefig('mwft_time.png')

def plot_mwft_sd(mwfts_norm):
    means, sds = [], []
    for mwfts in mwfts_norm:
        means.append(mean(mwfts))
        sds.append(sd(mwfts))

    print(means, sds)
    fig, ax = plt.subplots()
    ax.scatter(means, sds)
    ax.set_xlabel('Średnie znormalizowane MWFT')
    ax.set_ylabel('Odchylenie standardowe')
    ax.set_title('Odchylenie standardowe w zależności od MWFT')

    plt.savefig('mwft_sd.png')

def read_local_search_log(filepath):
    times, mwfts = [], []
    with open(filepath) as file:
        file.readline()
        for line in file:
            nums = line.split()
            times.append(float(nums[0]))
            mwfts.append(float(nums[1]))
    # The last line has finish time
    return times[:-1], mwfts[:-1]

def read_evaluator_log(filepath):
    BERTHS_PREFIX = 'Evaluating: '
    HEADER_PREFIX = 'MWFT'
    berths, mwfts_norm, mwfts, lower_bounds = [], [], [], []
    with open(filepath) as file:
        for line in file:
            if line.startswith(BERTHS_PREFIX):
                berths_line = line[len(BERTHS_PREFIX):].split()
                berths.append([int(i) for i in berths_line])

                mwfts_norm.append([])
                mwfts.append([])
                lower_bounds.append([])
            elif line.startswith(HEADER_PREFIX):
                pass
            else:
                score_line = line.split()
                mwfts_norm[-1].append(float(score_line[0]))
                mwfts[-1].append(float(score_line[1]))
                lower_bounds[-1].append(float(score_line[2]))
    print(len(berths), len(mwfts_norm), len(mwfts), len(lower_bounds))
    return berths, mwfts_norm, mwfts, lower_bounds

if __name__ == "__main__":
    times, mwfts = read_local_search_log('local_search.log')
    plot_mwft_time(times, mwfts)
    # _, mwfts_norm, _, _ = read_evaluator_log('sample_output_20proc_100inst_lh/logs/evaluator.log')
    _, mwfts_norm, _, _ = read_evaluator_log('evaluator.log')
    plot_mwft_sd(mwfts_norm)
