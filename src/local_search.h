#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>

class MoveGenerator {
public:
    MoveGenerator(int quay_length, const std::vector<int> &berth_lenghts);

    std::vector<std::vector<int>> get_neighbourhood(const std::vector<int> &berth_frequencies);
private:
    int quay_length;
    std::vector<int> berth_lengths;

    std::vector<int> try_split(int i, const std::vector<int> &berths);
    std::vector<int> try_merge(int i, int j, const std::vector<int> &berths);
    std::vector<int> add_longest(std::vector<int> &berths);
    bool is_valid(int i, const std::vector<int> &berths);
};

class LocalSearch {
public:
    void solve();
private:
    std::vector<int> berth_frequencies;
    std::vector<int> berth_lengths;
};

#endif
