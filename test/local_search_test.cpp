#include <vector>
#include <nlohmann/json.hpp>
#include "local_search.h"
#include "evaluator_interface.h"

#include "gtest/gtest.h"
using namespace std;
using json = nlohmann::json;

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
    expected = {{1, 0, 1, 1}, {0, 2, 0, 1}};
    EXPECT_EQ(expected, neigh);
}

class MockEvaluator : public EvaluatorInterface {
public:
    double evaluate(const std::vector<int> &berth_frequencies, 
                    const std::vector<int> &berth_lengths) {
        return 2;
    }
};

TEST(MoveGeneratorTest, test_solve) {
    MockEvaluator evaluator;
    int quay_length = 1000;
    vector<int> berth_lengths = {50, 100, 200, 300, 400};

    LocalSearch solver(quay_length, berth_lengths, evaluator);
    auto result = solver.solve();

    ASSERT_EQ(berth_lengths.size(), result.size());
    EXPECT_GE(result.back(), 1);

    int sum = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        sum += result[i] * berth_lengths[i];
    }
    EXPECT_LE(sum, quay_length);
}
