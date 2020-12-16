#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include "evaluator_interface.h"

class MoveGenerator {
public:
    MoveGenerator(int quay_length, const std::vector<int> &berth_lenghts);

    std::vector<std::vector<int>> get_neighborhood(const std::vector<int> &berth_frequencies);
private:
    int quay_length;
    std::vector<int> berth_lengths;

    std::vector<std::vector<int>> try_split(int i, const std::vector<int> &berths);
    std::vector<int> try_merge(int i, int j, const std::vector<int> &berths);
    void add_longest(std::vector<int> &berths);
    bool is_valid(std::size_t i, const std::vector<int> &berths);
};

class LocalSearch {
public:
    LocalSearch(int quay_length, const std::vector<int> &berth_lengths,
        EvaluatorInterface &evaluator, int max_restarts = 1, int max_time = 86400,
        const std::vector<int> &initial_solution = std::vector<int>());

    std::vector<int> solve();
    double score() { return final_score; }

private:
    int quay_length;
    std::vector<int> berth_lengths;
    EvaluatorInterface &evaluator;

    double final_score;

    std::vector<int> initial_solution;
    int max_restarts;
    std::chrono::seconds max_time;

    std::ofstream log;

    std::vector<int> initial_solution_longest();
    std::vector<int> initial_solution_random();

    void log_better_solution(const std::chrono::time_point<std::chrono::system_clock> &start_time, double mwft);
    bool should_exit(const std::chrono::time_point<std::chrono::system_clock> &start_time) {
        auto now = std::chrono::system_clock::now();
        return now - start_time >= max_time;
    }
};


#endif
