#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>

class MoveGenerator {
public:
    std::vector<int> next() {
        return {42};
    }
private:

};

class LocalSearch {
public:
    void solve();
private:
    std::vector<int> berth_frequencies;
    std::vector<int> berth_lengths;
    MoveGenerator move_generator;
};

#endif
