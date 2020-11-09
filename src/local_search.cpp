#include "local_search.h"
#include <vector>
#include "evaluator_interface.h"
using namespace std;

MoveGenerator::MoveGenerator(int quay_length, const vector<int> &berth_lengths)
    : quay_length(quay_length), berth_lengths(berth_lengths) { }

vector<vector<int>> MoveGenerator::get_neighborhood(const vector<int> &berth_frequencies) {
    vector<vector<int>> result;
    for (size_t i = 0; i < berth_frequencies.size(); ++i) {
        if (!is_valid(i, berth_frequencies)) {
            continue;
        }
        auto b = try_split(i, berth_frequencies);
        if (b.size() != 0) {
            result.push_back(b);
        }
    }

    for (size_t i = 0; i < berth_frequencies.size(); ++i) {
        for (size_t j = i; j < berth_frequencies.size(); ++j) {
            if (!is_valid(i, berth_frequencies) || !is_valid(j, berth_frequencies)) {
                continue;
            }
            if (i == j) {
                if (berth_frequencies[i] < 2) {
                    continue;
                }
                if (i == berth_frequencies.size() - 1 && berth_frequencies[i] < 3) {
                    continue;
                }
            }
            auto b = try_merge(i, j, berth_frequencies);
            if (b.size() != 0) {
                result.push_back(b);
            }
        }
    }
    return result;
}

vector<int> MoveGenerator::try_split(int i, const vector<int> &berths) {
    for (int k = i - 1; k >= 0; --k) {
        if (2 * berth_lengths[k] <= berth_lengths[i]) {
            auto result = berths;
            --result[i];
            result[k] += 2;
            add_longest(result);
            return result;
        }
    }
    return { };
}

vector<int> MoveGenerator::try_merge(int i, int j, const vector<int> &berths) {
    int sum = berth_lengths[i] + berth_lengths[j];
    for (int k = berth_lengths.size() - 1; k >= 0; --k) {
        if (berth_lengths[k] <= sum) {
            auto result = berths;
            --result[i];
            --result[j];
            ++result[k];
            add_longest(result);
            return result;
        }
    }
    return { };
}

void MoveGenerator::add_longest(vector<int> &berths) {
    int length_left = quay_length;
    for (size_t i = 0; i < berths.size(); ++i) {
        length_left -= berths[i] * berth_lengths[i];
    }
    for (int i = berths.size() - 1; i >= 0; --i) {
        if (berth_lengths[i] <= length_left) {
            ++berths[i];
            return;
        }
    }
}

bool MoveGenerator::is_valid(size_t i, const vector<int> &berths) {
    if (i == berths.size() - 1) {
        return berths[i] >= 2;
    }
    return berths[i] >= 1;
}

LocalSearch::LocalSearch(int quay_length, const vector<int> &berth_lengths, EvaluatorInterface &evaluator)
    : quay_length(quay_length), berth_lengths(berth_lengths), evaluator(evaluator) { }

vector<int> LocalSearch::solve() {
    MoveGenerator moveGenerator(quay_length, berth_lengths);
    vector<int> berth_frequencies = initial_solution();
    auto best = berth_frequencies;
    int best_eval = evaluator.evaluate(berth_frequencies, berth_lengths);

    bool found_better = true;
    while (found_better) {
        found_better = false;
        auto neighborhood = moveGenerator.get_neighborhood(berth_frequencies);
        for (vector<int> &berths : neighborhood) {
            int eval = evaluator.evaluate(berths, berth_lengths);
            if (eval < best_eval) {
                best = berths;
                best_eval = eval;
                berth_frequencies = berths;
                found_better = true;
                break;
            }
        }
    }
    return best;
}

std::vector<int> LocalSearch::initial_solution() {
    vector<int> berths(berth_lengths.size(), 0);
    berths.back() = 1;
    int length_left = quay_length - berth_lengths.back();
    for (int i = berths.size() - 1; i >= 0; --i) {
        int added = length_left / berth_lengths[i];
        length_left = length_left % berth_lengths[i];
        berths[i] += added;
    }
    return berths;
}
