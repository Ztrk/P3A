#include "local_search.h"
#include <chrono>
#include <iostream>
#include <random>
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
            result.insert(result.end(), b.begin(), b.end());
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

/*
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
*/

vector<vector<int>> MoveGenerator::try_split(int i, const vector<int> &berths) {
    int longest = 0;
    vector<pair<int, int>> best;
    for (int j = 0; j < i && berth_lengths[j] * 2 <= berth_lengths[i]; ++j) {
        for (int k = 0; k < i; ++k) {
            int length = berth_lengths[j] + berth_lengths[k];
            if (length == longest) {
                best.push_back({j, k});
            }
            else if (length <= berth_lengths[i] && length > longest) {
                longest = length;
                best.clear();
                best.push_back({j, k});
            }
        }
    }

    vector<vector<int>> result;
    for (auto &best_pair : best) {
        auto new_berths = berths;
        --new_berths[i];
        new_berths[best_pair.first] += 1;
        new_berths[best_pair.second] += 1;
        add_longest(new_berths);
        result.push_back(new_berths);
    }
    return result;
}

vector<int> MoveGenerator::try_merge(int i, int j, const vector<int> &berths) {
    int sum = berth_lengths[i] + berth_lengths[j];
    for (int k = berth_lengths.size() - 1; k > i && k > j; --k) {
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
    auto start_time = chrono::system_clock::now();

    MoveGenerator moveGenerator(quay_length, berth_lengths);
    vector<int> berth_frequencies = initial_solution_random();
    auto best = berth_frequencies;
    double best_eval = evaluator.evaluate(berth_frequencies, berth_lengths);
    log_better_solution(start_time, best_eval);

    bool found_better = true;
    while (found_better) {
        found_better = false;
        auto neighborhood = moveGenerator.get_neighborhood(berth_frequencies);
        for (vector<int> &berths : neighborhood) {
            double eval = evaluator.evaluate(berths, berth_lengths);
            if (eval < best_eval) {
                best = berths;
                best_eval = eval;
                berth_frequencies = berths;
                found_better = true;

                log_better_solution(start_time, best_eval);
                break;
            }
        }
    }
    final_score = best_eval;
    return best;
}

vector<int> LocalSearch::initial_solution_longest() {
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

vector<int> LocalSearch::initial_solution_random() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> index_dist(0, berth_lengths.size() - 1);

    vector<int> berths(berth_lengths.size());
    berths.back() = 1;
    int length_left = quay_length - berth_lengths.back();

    int i = index_dist(gen);
    while (berth_lengths[i] <= length_left) {
        ++berths[i];
        length_left -= berth_lengths[i];
        i = index_dist(gen);
    }
    return berths;
}

void LocalSearch::log_better_solution(std::chrono::time_point<std::chrono::system_clock> start_time, double mwft) {
    auto time = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(time - start_time);
    log << duration.count() / 1000.0 << " s, MWFT: " << mwft << endl;
}
