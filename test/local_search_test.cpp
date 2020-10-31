#include "local_search.h"

#include "gtest/gtest.h"
#include <vector>
using namespace std;

TEST(MoveGeneratorTest, test_move_generator) {
    vector<int> lengths = {100, 200, 300, 400};
    int quay_length = 200;
    MoveGenerator generator(quay_length, lengths);

    auto neigh = generator.get_neighborhood({0, 1, 0, 0});
    vector<vector<int>> expected = {{2, 0, 0, 0}};
    EXPECT_EQ(expected, neigh);

    neigh = generator.get_neighborhood({2, 0, 0, 0});
    expected = {{0, 1, 0, 0}};
    EXPECT_EQ(expected, neigh);

    neigh = generator.get_neighborhood({1, 1, 0, 0});
    expected = {{3, 0, 0, 0}, {0, 0, 1, 0}};
    EXPECT_EQ(expected, neigh);

    neigh = generator.get_neighborhood({0, 0, 0, 1});
    expected = {};
    EXPECT_EQ(expected, neigh);

    neigh = generator.get_neighborhood({0, 0, 0, 2});
    expected = {{0, 2, 0, 1}};
    EXPECT_EQ(expected, neigh);
}

TEST(MoveGeneratorTest, test_solve) {
    // LocalSearch solver;
    // solver.solve();
    EXPECT_EQ(2, 2);
}
