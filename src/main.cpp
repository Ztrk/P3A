#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <mpi.h>
#include <nlohmann/json.hpp>
#include "evaluator.h"
#include "instance_generator.h"
#include "mpi_evaluator.h"
#include "local_search.h"

using namespace std;
using nlohmann::json;

int main(int argc, char *argv[]) {
    random_device rd;
    srand(rd());

    json config;
    ifstream config_file("p3a_config.json");
    config_file >> config;
    config_file.close();

    MPI_Init(&argc, &argv);

    int pid, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    string instances_path = config["folder_with_instances"].get<string>();
    bool read_instances_from_file = config["read_ships_from_file"].get<bool>();
    int n_instances = config["n_instances"].get<int>();

    InstanceGenerator generator(instances_path, read_instances_from_file);
    Evaluator evaluator(config["bap_algorithms"], generator);
    MpiEvaluator mpi_evaluator(n_instances, evaluator);

    if (pid == 0) {
        int quay_length = 0;
        vector<int> berth_lengths;

        if (argc >= 3 && strcmp(argv[1], "-i") == 0) {
            ifstream file(argv[2]);
            int tmp;
            file >> quay_length;
            while (file >> tmp) {
                berth_lengths.push_back(tmp);
            }
        }
        else {
            quay_length = config["quay_length"].get<int>();
            berth_lengths = config["berths"].get<vector<int>>();
        }
        sort(berth_lengths.begin(), berth_lengths.end());

        LocalSearch solver(quay_length, berth_lengths, mpi_evaluator);
        auto result = solver.solve();

        cout << "\nSolution found: \n";
        for (size_t i = 0; i < berth_lengths.size(); ++i) {
            cout << berth_lengths[i] << ' ';
        }
        cout << '\n';
        for (size_t i = 0; i < result.size(); ++i) {
            cout << result[i] << ' ';
        }
        cout << '\n';
        cout << solver.score() << endl;

        mpi_evaluator.stop_listeners();
    }
    else {
        mpi_evaluator.listen();
    }

    MPI_Finalize();

    return 0;
}
