#include "evaluator.h"
#include "local_search.h"

#include <vector>
using namespace std;

int main() {
    Evaluator evaluator;
    int quay_length = 1000;
    vector<int> berth_lengths = {50, 100, 200, 300, 400};

    LocalSearch solver(quay_length, berth_lengths, evaluator);
    solver.solve();
    return 0;
}
