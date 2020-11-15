#include <fstream>
#include <iostream>
#include <vector>
#include <mpi.h>
#include <nlohmann/json.hpp>
#include "mpi_evaluator.h"
#include "local_search.h"

using namespace std;
using nlohmann::json;

int main(int argc, char *argv[]) {
    json config;
    ifstream config_file("p3a_config.json");
    config_file >> config;
    config_file.close();

    MPI_Init(&argc, &argv);

    int pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    Evaluator evaluator(config["bap_algorithms"]);
    MpiEvaluator mpi_evaluator(config["n_instances"].get<int>(), evaluator);
    if (pid == 0) {
        int quay_length = config["quay_length"].get<int>();
        vector<int> berth_lengths = config["berths"].get<vector<int>>();

        LocalSearch solver(quay_length, berth_lengths, mpi_evaluator);
        auto result = solver.solve();

        for (size_t i = 0; i < berth_lengths.size(); ++i) {
            cout << berth_lengths[i] << ' ';
        }
        cout << endl;
        for (size_t i = 0; i < result.size(); ++i) {
            cout << result[i] << ' ';
        }
        cout << endl;

        mpi_evaluator.stop_listeners();
    }
    else {
        mpi_evaluator.listen();
    }

    MPI_Finalize();

    return 0;
}
