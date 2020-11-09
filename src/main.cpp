#include "evaluator.h"
#include "local_search.h"

#include <vector>
#include <iostream>
using namespace std;

int main() {
    Evaluator evaluator;
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

    return 0;
}
