#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <chrono>
#include <fstream>
#include <iostream>
#include <ostream>
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
    LocalSearch(int quay_length, const std::vector<int> &berth_lengths, EvaluatorInterface &evaluator);

    std::vector<int> solve();
    double score() { return final_score; }

private:
    int quay_length;
    std::vector<int> berth_lengths;

    EvaluatorInterface &evaluator;

    double final_score;

    std::ofstream file = std::ofstream("local_search.log");
    std::ostream &log = file;

    std::vector<int> initial_solution_longest();
    std::vector<int> initial_solution_random();

    void log_better_solution(std::chrono::time_point<std::chrono::system_clock> start_time, double mwft);
};


#endif
