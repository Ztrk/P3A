#include <vector>
#include <iostream>
#include <mpi.h>
#include "mpi_evaluator.h"
#include "local_search.h"

using namespace std;

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    MpiEvaluator evaluator(8);
    if (pid == 0) {
        int quay_length = 1000;
        vector<int> berth_lengths = {50, 100, 200, 300, 400};

        LocalSearch solver(quay_length, berth_lengths, evaluator);
        auto result = solver.solve();

        for (int i = 0; i < berth_lengths.size(); ++i) {
            cout << berth_lengths[i] << ' ';
        }
        cout << endl;
        for (int i = 0; i < result.size(); ++i) {
            cout << result[i] << ' ';
        }
        cout << endl;
    }
    else {
        evaluator.listen();
    }

    MPI_Finalize();

    return 0;
}
