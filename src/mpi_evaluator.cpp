#include "mpi_evaluator.h"
#include <iostream>
#include <vector>
#include <mpi.h>
using namespace std;

double MpiEvaluator::evaluate(const vector<int> &berth_frequencies, 
                              const vector<int> &berth_lengths) {

    double mwft_sum = 0;
    int pid, np;
    MPI_Status status;

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    int instances_per_process = num_instances/np;

    for (int i = 1; i < np; ++i) {
        MPI_Send(&instances_per_process, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
    }

    cout << "Pid: " << pid << endl;
    cout << instances_per_process << endl;

    evaluator.set_num_instances(instances_per_process);
    double mwft = evaluator.evaluate(berth_frequencies, berth_lengths);

    cout << "mwft: " << mwft << endl;

    mwft_sum += mwft;
    for (int i = 1; i < np; i++)
    {
        MPI_Recv(&mwft, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 
                 MPI_TAG, MPI_COMM_WORLD, &status);
        mwft_sum += mwft;
    }                         

    return mwft_sum;
}

void MpiEvaluator::listen() {
    int n_instances = 0;
    MPI_Status status;

    int pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    cout << pid << " listening" << endl;

    while (true) {
        MPI_Recv(&n_instances, 1, MPI_INT, ROOT, MPI_TAG, MPI_COMM_WORLD, &status);


        if (n_instances == EXIT) {
            break;
        }

        evaluator.set_num_instances(n_instances);
        vector<int> berth_frequencies = {0, 1, 2};
        vector<int> berth_lengths = {100, 200, 400};
        double mwft = evaluator.evaluate(berth_frequencies, berth_lengths);
        cout << "Listener " << pid << ' ' << mwft << endl;

        MPI_Send(&mwft, 1, MPI_DOUBLE, ROOT, MPI_TAG, MPI_COMM_WORLD);
    }
}
