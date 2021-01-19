import matplotlib.pyplot as plt

OUTPUT_FOLDER = 'output/si-singapur/speedup/'

def mean(x):
    return sum(x) / len(x)

def plot_speedup_instances(n_instances, times_n_proc, times_one_proc):
    # Preprocessing
    n_instances.reverse()
    times_one_proc.reverse()
    times_n_proc.reverse()
    
    print(times_one_proc, times_n_proc)

    times_1 = [mean(times) for times in times_one_proc]
    times_n = [mean(times) for times in times_n_proc]
    y = [y1/yn for y1, yn in zip(times_1, times_n) ]
    print(y)

    fig, ax = plt.subplots()
    ax.plot(n_instances, y)
    ax.set_xlabel('Liczba instancji')
    ax.set_ylabel('Przyspieszenie')
    ax.set_title('Przyspieszenie dla 5 procesorów w zależności od liczby instancji')

    plt.savefig(OUTPUT_FOLDER + 'speedup_instances.png')


def plot_speedup_processors(processors, times):
    # Preprocessing
    speedup = [times[0][0]/time[0] for time in times]
    print(speedup)

    fig, ax = plt.subplots()
    ax.plot(processors, speedup)
    ax.set_xlabel('Liczba procesorów')
    ax.set_ylabel('Przyspieszenie')
    ax.set_title('Przyspieszenie dla 100 instancji w zależności od liczby procesorów')

    plt.savefig(OUTPUT_FOLDER + 'speedup_processors.png')


def read_runtimes(filepath, times_per_class=5):
    n_instances = []
    times = []
    with open(filepath) as file:
        numbers = file.read().split()
        for i, line in enumerate(numbers):
            if i % (times_per_class + 1) == 0:
                n_instances.append(int(line))
                times.append([])
            else:
                times[-1].append(float(line))
    return n_instances, times


if __name__ == "__main__":
    instances1, times_proc1 = read_runtimes('hb_rt_sh_si-results/1proc50to10inst_step5-si.txt', 2)
    instances5, times_proc5 = read_runtimes('hb_rt_sh_si-results/5proc50to10inst_step5-si.txt', 3)
    instances10, times_proc10 = read_runtimes('hb_rt_sh_si-results/10proc50to10inst_step10-si.txt', 3)
    # times_proc1 = [times for i, times in enumerate(times_proc1) if i % 2 == 0]
    plot_speedup_instances(instances5, times_proc5, times_proc1)

    # processors, times = read_runtimes('output/speedup/100instances_1to16proc.txt', 1)
    # processors, times = read_runtimes('output/speedup/processes_times_2.txt', 1)
    # plot_speedup_processors(processors, times)
