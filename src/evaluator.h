#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "evaluator_interface.h"
#include "instance_generator.h"

class Evaluator : public EvaluatorInterface {
private:
    const nlohmann::json &bap_algorithms;
    float lower_bound;
    std::vector<generator::ship> ships_from_instance;
    float MWFT_BAP;
    float mwft_instance_sum = 0;
    float mwft_from_one_processor = 0;
    int num_of_instances = 1;

    std::vector<int> berth_frequencies;
    std::vector<int> berth_lengths;

    void init_bap(const nlohmann::json &options);
public:
    Evaluator(const nlohmann::json &bap_algorithms) : bap_algorithms(bap_algorithms) { }

    void set_num_instances(int instances) {
        this->num_of_instances = instances;
    }

    float calculate_lower_bound();

    void reset_values();

    void restore_ships_from_instance();

    void initialize();

    float schedule();

    float calculateMWFT();

    double evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths);

};

#endif
