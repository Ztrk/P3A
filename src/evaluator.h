#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "evaluator_interface.h"
#include "instance_generator.h"
#include "ship.h"

class Evaluator : public EvaluatorInterface {
public:
    Evaluator(const nlohmann::json &bap_algorithms) : bap_algorithms(bap_algorithms) { }

    void set_num_instances(int instances) {
        this->num_of_instances = instances;
    }

    double evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths);
    std::vector<double> calculate_scores(const std::vector<int> &berth_frequencies, 
                                         const std::vector<int> &berth_lengths);
    double aggregate(const std::vector<double> &scores);

private:
    using ScheduleFunction = double (*)(const std::vector<ship> &, const std::vector<berth> &);
    const nlohmann::json &bap_algorithms;
    int num_of_instances = 1;

    ScheduleFunction init_bap(const nlohmann::json &options);
    double calculate_lower_bound(const std::vector<ship> &ships);
    std::vector<berth> berths_to_list(const std::vector<int> &berth_frequencies, 
                                      const std::vector<int> &berth_lengths);
};

#endif
