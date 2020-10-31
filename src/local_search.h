#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include "evaluator.h"

class MoveGenerator {
public:
    MoveGenerator(int quay_length, const std::vector<int> &berth_lenghts);

    std::vector<std::vector<int>> get_neighborhood(const std::vector<int> &berth_frequencies);
private:
    int quay_length;
    std::vector<int> berth_lengths;

    std::vector<int> try_split(int i, const std::vector<int> &berths);
    std::vector<int> try_merge(int i, int j, const std::vector<int> &berths);
    void add_longest(std::vector<int> &berths);
    bool is_valid(std::size_t i, const std::vector<int> &berths);
};

class LocalSearch {
public:
    LocalSearch(int quay_length, const std::vector<int> &berth_lengths, Evaluator evaluator);

    std::vector<int> solve();
private:
    int quay_length;
    std::vector<int> berth_lengths;

    Evaluator evaluator;

    std::vector<int> initial_solution();
};

#endif
