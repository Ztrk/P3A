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
        int n_berths = berth_lengths.size();
        MPI_Send(&n_berths, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
        MPI_Send(berth_frequencies.data(), n_berths, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
        MPI_Send(berth_lengths.data(), n_berths, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
    }

    evaluator.set_num_instances(instances_per_process);
    double mwft = evaluator.evaluate(berth_frequencies, berth_lengths);

    mwft_sum += mwft;
    for (int i = 1; i < np; i++)
    {
        MPI_Recv(&mwft, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 
                 MPI_TAG, MPI_COMM_WORLD, &status);
        mwft_sum += mwft;
    }                         
    cout << "mwft: " << mwft_sum << endl;

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
        
        int n_berths;
        MPI_Recv(&n_berths, 1, MPI_INT, ROOT, MPI_TAG, MPI_COMM_WORLD, &status);
        vector<int> berth_frequencies(n_berths);
        vector<int> berth_lengths(n_berths);

        MPI_Recv(berth_frequencies.data(), n_berths, MPI_INT, ROOT, MPI_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(berth_lengths.data(), n_berths, MPI_INT, ROOT, MPI_TAG, MPI_COMM_WORLD, &status);

        evaluator.set_num_instances(n_instances);
        double mwft = evaluator.evaluate(berth_frequencies, berth_lengths);
        cout << "Listener " << pid << ' ' << mwft << endl;

        MPI_Send(&mwft, 1, MPI_DOUBLE, ROOT, MPI_TAG, MPI_COMM_WORLD);
    }
}

void MpiEvaluator::stop_listeners() {
    int n;
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    int msg = EXIT;
    for (int i = 1; i < n; ++i) {
        MPI_Send(&msg, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
    }
}
