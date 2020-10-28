#include "local_search.h"
#include <iostream>
using namespace std;

MoveGenerator::MoveGenerator(int quay_length, const vector<int> &berth_lengths)
    : quay_length(quay_length), berth_lengths(berth_lengths) { }

vector<vector<int>> MoveGenerator::get_neighbourhood(const vector<int> &berth_frequencies) {
    vector<vector<int>> result;
    for (int i = 0; i < berth_frequencies.size(); ++i) {
        if (!is_valid(i, berth_frequencies)) {
            continue;
        }
        auto b = try_split(i, berth_frequencies);
        if (b.size() != 0) {
            result.push_back(b);
        }
    }

    for (int i = 0; i < berth_frequencies.size(); ++i) {
        for (int j = i; j < berth_frequencies.size(); ++j) {
            if (!is_valid(i, berth_frequencies) || !is_valid(j, berth_frequencies)) {
                continue;
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

std::vector<int> MoveGenerator::try_merge(int i, int j, const vector<int> &berths) {
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

std::vector<int> MoveGenerator::add_longest(std::vector<int> &berths) {
    // TODO
    return berths;
}

bool MoveGenerator::is_valid(int i, const vector<int> &berths) {
    if (i == berths.size() - 1) {
        return berths[i] >= 2;
    }
    return berths[i] >= 1;
}

void LocalSearch::solve() {
    cout << "Solving" << endl;
}
