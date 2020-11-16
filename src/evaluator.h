#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "evaluator_interface.h"
#include "instance_generator.h"

class Evaluator : public EvaluatorInterface {
public:
    Evaluator(const nlohmann::json &bap_algorithms) : bap_algorithms(bap_algorithms) { }

    void set_num_instances(int instances) {
        this->num_of_instances = instances;
    }

    double evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths);
    std::vector<double> calculate_scores(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths);
    double aggregate(const std::vector<double> &scores);

private:
    const nlohmann::json &bap_algorithms;
    std::vector<generator::ship> ships_from_instance;
    int num_of_instances = 1;

    std::vector<int> berth_frequencies;
    std::vector<int> berth_lengths;

    void init_bap(const nlohmann::json &options);
    float calculate_lower_bound();
    void reset_values();
    void restore_ships_from_instance();
    void initialize();
    float schedule();
};

#endif
