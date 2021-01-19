import matplotlib.pyplot as plt
from math import sqrt

def mean(x):
    return sum(x) / len(x)

def sd(x):
    avg = mean(x)
    square_sum = sum([(xi - avg)**2 for xi in x])
    return sqrt(square_sum / (len(x) - 1))

def plot_mwft_time(times, mwft):
    # The last line has finish time, which is unnecessary
    times[-1].pop()
    mwft[-1].pop()

    for i in range(0, len(times)):
        times[i] = [time - times[i][0] for time in times[i]]

    fig, ax = plt.subplots()
    for x, y in zip(times, mwft):
        ax.plot(x, y, '.-')
    ax.set_xlabel('Czas[s]')
    ax.set_ylabel('Średnie znormalizowane MWFT')
    ax.set_title('MWFT w zależności od czasu')

    plt.savefig('mwft_time.png')

def plot_mwft_sd(berths, mwfts_norm):
    means, sds = [], []
    data = []
    for i, mwfts in enumerate(mwfts_norm):
        # mwfts_filtered = [min(mwfts[i:i + 4]) for i in range(0, len(mwfts), 4)]
        # print(mwfts_filtered)
        means.append(mean(mwfts))
        sds.append(sd(mwfts))
        # data.append((means[-1], sds[-1], berths[i]))
        # print(f'{means[-1]:.6f} {sds[-1]:.6f} {berths[i]}')

    # data.sort(key=lambda x: -x[1])
    # for mean_val, sd_val, berth in data:
        # print(f'{mean_val:.6f} {sd_val:.6f} {berth}')

    fig, ax = plt.subplots()
    ax.scatter(means, sds)
    ax.set_xlabel('Średnie znormalizowane MWFT')
    ax.set_ylabel('Odchylenie standardowe')
    ax.set_title('Odchylenie standardowe w zależności od MWFT')

    plt.savefig('mwft_sd.png')

def read_local_search_log(filepath):
    RESTART_PREFIX = 'Finding'
    HEADER_PREFIX = 'time'
    times, mwfts = [], []
    with open(filepath) as file:
        for line in file:
            if line.startswith(RESTART_PREFIX):
                pass
            elif line.startswith(HEADER_PREFIX):
                times.append([])
                mwfts.append([])
            else:
                nums = line.split()
                times[-1].append(float(nums[0]))
                mwfts[-1].append(float(nums[1]))
    return times, mwfts

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
    return berths, mwfts_norm, mwfts, lower_bounds

if __name__ == "__main__":
    # output_folder = 'output/le_havre/run2_10000/'
    output_folder = ''
    times, mwfts = read_local_search_log(output_folder + 'local_search.log')
    plot_mwft_time(times, mwfts)
    berths, mwfts_norm, _, _ = read_evaluator_log(output_folder + 'evaluator.log')
    plot_mwft_sd(berths, mwfts_norm)
