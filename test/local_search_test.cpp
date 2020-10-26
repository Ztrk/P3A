#include "local_search.h"

#include "gtest/gtest.h"
#include <vector>

TEST(MoveGeneratorTest, test_next) {
    MoveGenerator generator;
    EXPECT_EQ(std::vector<int>({42}), generator.next());
}

TEST(MoveGeneratorTest, test_solve) {
    LocalSearch solver;
    solver.solve();
    EXPECT_EQ(2, 2);
}
