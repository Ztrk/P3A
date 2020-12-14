# P3A
Probabilistic port planning algorithm - praca inżynierska

### Kompilacja i uruchomienie

Kod jest kompilowany za pomocą `cmake`:

    mkdir build
    cmake -S . -B build
    cd build
    make

Aby skompilować program potrzebne są biblioteki MPI oraz `nlohmann/json`.

Program można uruchomić komendą `./build/p3a` lub `mpirun -np N ./build/p3a`. Konfiguracja znajduje się w plikach `p3a_config.json` oraz `lh_instance.json` - konfiguracja generatora instancji. Program wypisuje logi do plików `evaluator.log` i `local_search.log`.

#### Konfiguracja

Konfiguracja algorytmu znajduje się w pliku `p3a_config.json`. Przykładowy plik jest dołączony. Dostępne opcje oraz ich znaczenie jest następujące:

- `quay_length` - Całkowita długość nabrzeża
- `berths` - Dopuszczalne długości nabrzeży
- `n_instances` - Liczba instancji, na których podział jest oceniany. Jeśli nie jest wielokrotnością liczby procesów to jest zmniejszana do nabliższej wielokrotności.
- `read_ships_from_file` - Jeśli jest ustawiona na `true` instancje (statki) czytane są z pliku, w przeciwnym wypadku są generowane.
- `folder_with_instances` - Ścieżka do folderu z którego czytane i zapisywane są instancje. Instancje są zapisywane pod nazwą `used-instanceN.txt`, a odczytywane z plików `instanceN.txt`
- `bap_algorithms` - Tablica używanych algorytmów BAP. Pierwsze pole struktur to `algorithm` - możliwe są wartości `greedy` i `ils`. Kolejne wartości to opcje danego algorytmu. Dla `greedy` są to: `scheduling_policy`, `future_arrivals`, a dla `ils` są to `destruction_method` i `destruction_fraction`.

Algortym obsługuje opcję `-i` umożliwiającą podanie pliku z długościami nabrzeży. Wtedy `quay_length` i `berths` w pliku `p3a_config.json` są ignorowane.

Kod generatora instancji (`instance_generator.cpp`) i algorytmów BAP (`ils.cpp`, `greedy.cpp`) jest autorstwa Jakuba Wawrzyniaka.

#### PCSS - uruchomienie

Podstawową wersję skryptu -  `p3ascr.sh` uruchamiamy w miejscu gdzie znajduję się program `p3a` za pomocą komendy `sbatch p3ascr.sh`. Program tworzy wstępną hierarchię plików podzieloną na foldery przypisując każdemu nazwę - podział nabrzeża (wektor frekwencji). Plik `file_system.py` dodatkowo tworzy kolejne podkatalogi - dotyczące instancji, a w nich wyniki poszczególnych algorytmów BAP w plikach `.txt`.

Druga wersja skryptu - `p3ascr_v2_const_p_diff_inst.sh` służy do pomiaru czasu wykonywania algorytmu przy tej samej liczbie procesorów (10) i różnej liczbie instancji (100-10). Uruchomić ją można w podobny sposób jak 1-szą wersje: `sbatch p3ascr_v2_const_p_diff_inst.sh`. Do parsowania wyników potrzebny jest skrypt `get_inst_time.py`, który pobiera ostatnią linię pliku wyjściowego `local_search.log` - czas wykonania programu dla danej liczby instancji.

### Pliki wyjściowe - druga wersja skryptu

Po uruchomieniu skryptu `p3ascr_v2_const_p_diff_inst.sh` powstaje plik wynikowy `10tasks_100to10inst.txt`, który zawiera cykliczne informacje zawierające - liczbę instancji i 5 wartości czasów wykonywanego algorytmu, każda informacja w osobnym wierszu. Przy obecności tego pliku można uruchomić skrypt `diff_inst_mean_alg_time.py` (python diff_inst_mean_alg_time.py), co daje ostatecznie w pliku wynikowym `mean_times_const_proc_diff_inst.txt` zestawienie: 'liczba instancji' oraz 'przyspieszenie dla danej liczby instancji' w odniesieniu do największej uwzględnianej liczby instancji (100) przy stałej ilości procesorów (--ntasks w skrypcie).



