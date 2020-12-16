import matplotlib.pyplot as plt

def mean(x):
    return sum(x) / len(x)

def plot_speedup_instances(n_instances, times_n_proc, times_one_proc):
    # Preprocessing
    n_instances.reverse()
    times_one_proc.reverse()
    times_n_proc.reverse()

    # FIXME: Until there are better results, add time for 100 instances/1 proc from Cyprian
    for i in range(3, 11):
        times_one_proc.append([i/10 * 1256] * 5)

    times_1 = [mean(sorted(times)[1:-1]) for times in times_one_proc]
    times_n = [mean(sorted(times)[1:-1]) for times in times_n_proc]
    y = [y1/yn for y1, yn in zip(times_1, times_n) ]
    print(y)

    fig, ax = plt.subplots()
    ax.plot(n_instances, y)
    ax.set_xlabel('Liczba instancji')
    ax.set_ylabel('Przyspieszenie')
    ax.set_title('Przyspieszenie dla 10 procesorów w zależności od liczby instancji')

    plt.savefig('output/speedup/speedup_instances.png')


def plot_speedup_processors(processors, times):
    # Preprocessing
    speedup = [times[0][0]/time[0] for time in times]
    print(speedup)

    fig, ax = plt.subplots()
    ax.plot(processors, speedup)
    ax.set_xlabel('Liczba procesorów')
    ax.set_ylabel('Przyspieszenie')
    ax.set_title('Przyspieszenie dla 100 instancji w zależności od liczby procesorów')

    plt.savefig('output/speedup/speedup_processors.png')


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
    instances, times_one_proc = read_runtimes('output/speedup/1proc_20to10inst.txt', 5)
    instances, times_n_proc = read_runtimes('output/speedup/10proc_100to10inst.txt', 5)
    plot_speedup_instances(instances, times_n_proc, times_one_proc)

    processors, times = read_runtimes('output/speedup/100instances_1to16proc.txt', 1)
    plot_speedup_processors(processors, times)
