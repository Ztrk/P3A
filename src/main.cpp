#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
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
    if (config_file.fail()) {
        throw runtime_error("Could not open 'p3a_config.json'.");
    }

    config_file >> config;
    config_file.close();

    int res = MPI_Init(&argc, &argv);
    if (res != 0) {
        throw runtime_error(string("MPI_Init failed with code ") + to_string(res) + ".");
    }

    int pid, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    string instances_path = config["folder_with_instances"].get<string>();
    bool read_instances_from_file = config["read_ships_from_file"].get<bool>();
    int n_instances;// = config["n_instances"].get<int>();

    if(argc >= 5 && strcmp(argv[3], "-t") == 0) {
        n_instances = stoi(argv[4]);
    }
    else {
        n_instances = config["n_instances"].get<int>();
    }

    InstanceGenerator generator(instances_path, read_instances_from_file);
    Evaluator evaluator(config["bap_algorithms"], generator);
    MpiEvaluator mpi_evaluator(n_instances, evaluator);

    if (pid == 0) {
        int quay_length = 0;
        vector<int> berth_lengths;
        vector<int> initial_solution = config.value("initial_solution", vector<int>());
        int max_restarts = config.value("max_restarts", 1);
        int max_time = config.value("time_limit", 86400);

        if (argc >= 3 && strcmp(argv[1], "-i") == 0) {
            ifstream file(argv[2]);
            if (file.fail()) {
                throw runtime_error(string("Could not open berths file ") + argv[2]);
            }

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

	//Poszukiwacze zaginionego hiperparametru
	double best_eval;
	bool hyperparameter_tune=config.value("hyperparameter_tune", false);
	vector<int> result;
	if (hyperparameter_tune){
		vector<int> hyperparameter_tuning_options=config["hyperparameter_tuning_options"].get<vector<int>>();
		int ho_len=hyperparameter_tuning_options.size();
		vector<int> mini_result;
		int best_f=-1;

		for (auto f: hyperparameter_tuning_options){
			LocalSearch solver(quay_length, berth_lengths, mpi_evaluator,
			    max_restarts, max_time/ho_len, vector<int>{}, f);
			auto mini_result = solver.solve();
			double eval=solver.score();
			if (best_f==-1 || eval<best_eval){
				result=mini_result;
				best_eval=eval;
				best_f=f;
			}
			cout << "Currently processed number of starts: " << f <<" with result "<< eval << endl;
		}
		cout << "The best number of starts is: " << best_f << endl;
	}

	else{
		int ls_starts=config.value("hyperparameter_ls_starts", 500);
		LocalSearch solver(quay_length, berth_lengths, mpi_evaluator,
		    max_restarts, max_time, initial_solution, ls_starts);
		result = solver.solve();
		best_eval=solver.score();
	}

        cout << "Solution found: \n";
        for (size_t i = 0; i < berth_lengths.size(); ++i) {
            cout << berth_lengths[i] << ' ';
        }
        cout << '\n';
        for (size_t i = 0; i < result.size(); ++i) {
            cout << result[i] << ' ';
        }
        cout << '\n';
        cout << best_eval << endl;

        mpi_evaluator.stop_listeners();
    }
    else {
        mpi_evaluator.listen();
    }

    MPI_Finalize();

    return 0;
}

