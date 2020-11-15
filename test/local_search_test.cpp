#include <vector>
#include <nlohmann/json.hpp>
#include "local_search.h"
#include "evaluator.h"

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
    expected = {{0, 2, 0, 1}};
    EXPECT_EQ(expected, neigh);
}

TEST(MoveGeneratorTest, test_solve) {
    json config;
    ifstream config_file("p3a_config.json");
    config_file >> config;
    config_file.close();

    Evaluator evaluator(config["bap_algorithms"]);
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
