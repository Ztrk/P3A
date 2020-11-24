#include "mpi_evaluator.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <mpi.h>
using namespace std;

double MpiEvaluator::evaluate(const vector<int> &berth_frequencies, 
                              const vector<int> &berth_lengths) {

    int pid, np;
    MPI_Status status;

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    int instances_per_process = num_instances/np;

    cout << "\nEvaluating: ";
    log << "Evaluating: ";
    for (size_t i = 0; i < berth_frequencies.size(); ++i) {
        cout << berth_frequencies[i] << ' ';
        log << berth_frequencies[i] << ' ';
    }
    cout << endl;
    log << '\n';

    for (int i = 1; i < np; ++i) {
        MPI_Send(&instances_per_process, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
        int n_berths = berth_lengths.size();
        MPI_Send(&n_berths, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
        MPI_Send(berth_frequencies.data(), n_berths, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
        MPI_Send(berth_lengths.data(), n_berths, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
    }

    evaluator.set_num_instances(instances_per_process);
    vector<double> scores = evaluator.calculate_scores(berth_frequencies, berth_lengths);
    vector<double> mwft_not_normalized = evaluator.mwft_not_normalized();
    vector<double> lower_bounds = evaluator.lower_bounds();

    int scores_per_instance = scores.size();
    for (int i = 1; i < np; i++)
    {
        vector<double> tmp(scores_per_instance);
        MPI_Recv(tmp.data(), scores_per_instance, MPI_DOUBLE, i, 
                 MPI_TAG, MPI_COMM_WORLD, &status);
        scores.insert(scores.end(), tmp.begin(), tmp.end());

        MPI_Recv(tmp.data(), scores_per_instance, MPI_DOUBLE, i, 
                 MPI_TAG, MPI_COMM_WORLD, &status);
        mwft_not_normalized.insert(mwft_not_normalized.end(), tmp.begin(), tmp.end());

        MPI_Recv(tmp.data(), scores_per_instance, MPI_DOUBLE, i, 
                 MPI_TAG, MPI_COMM_WORLD, &status);
        lower_bounds.insert(lower_bounds.end(), tmp.begin(), tmp.end());
    }

    double mwft_sum = evaluator.aggregate(scores);
    double sd = standard_deviation(scores);
    vector<double> quart = quartiles(scores);

    log << "MWFT(norm) MWFT LB" << '\n';
    for (size_t i = 0; i < scores.size(); ++i) {
        log << scores[i] << ' ' << mwft_not_normalized[i] << ' ' << lower_bounds[i] << '\n';
    }
    log.flush();

    cout << "Mean MWFT(norm): " << mwft_sum << '\n';
    cout << "Min, Max: " << min(scores) << ' ' << max(scores) << '\n';
    cout << "Quartiles: " << quart[0] << ' ' << quart[1] << ' ' << quart[2] << '\n';
    cout << "Std dev.: " << sd << '\n';
    cout << "IQR: " << quart[2] - quart[0] << endl;
    return mwft_sum;
}

void MpiEvaluator::listen() {
    int n_instances = 0;
    MPI_Status status;

    int pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    cout << "Process " << pid << " listening" << endl;

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
        vector<double> scores = evaluator.calculate_scores(berth_frequencies, berth_lengths);
        vector<double> mwft_not_normalized = evaluator.mwft_not_normalized();
        vector<double> lower_bounds = evaluator.lower_bounds();

        MPI_Send(scores.data(), scores.size(), MPI_DOUBLE, ROOT, MPI_TAG, MPI_COMM_WORLD);
        MPI_Send(mwft_not_normalized.data(), mwft_not_normalized.size(), MPI_DOUBLE, ROOT, MPI_TAG, MPI_COMM_WORLD);
        MPI_Send(lower_bounds.data(), lower_bounds.size(), MPI_DOUBLE, ROOT, MPI_TAG, MPI_COMM_WORLD);
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

double MpiEvaluator::standard_deviation(const vector<double> &mwft) {
    double mean = evaluator.aggregate(mwft);
    double variance = 0;
    for (size_t i = 0; i < mwft.size(); ++i) {
        variance += (mwft[i] - mean) * (mwft[i] - mean);
    }
    return sqrt(variance / (mwft.size() - 1));
}

vector<double> MpiEvaluator::quartiles(const vector<double> &mwft) {
    auto values = mwft;
    sort(values.begin(), values.end());
    int half = values.size()/2;
    int is_odd = values.size() % 2 != 0;
    return {
        median(vector<double>(values.begin(), values.begin() + half)),
        median(values),
        median(vector<double>(values.begin() + half + is_odd, values.end()))
    };
}

double MpiEvaluator::median(std::vector<double> x) {
    sort(x.begin(), x.end());
    int half = x.size()/2;
    return x.size() % 2 == 0 ? (x[half] + x[half - 1])/2 : x[half];
}

double MpiEvaluator::min(const std::vector<double> &x) {
    double result = INFINITY;
    for (size_t i = 0; i < x.size(); ++i) {
        result = std::min(x[i], result);
    }
    return result;
}

double MpiEvaluator::max(const std::vector<double> &x) {
    double result = -INFINITY;
    for (size_t i = 0; i < x.size(); ++i) {
        result = std::max(x[i], result);
    }
    return result;
}
