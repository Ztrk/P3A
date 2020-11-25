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
    using InstanceGenerator = std::vector<ship> (*)(int);

    Evaluator(const nlohmann::json &bap_algorithms, InstanceGenerator instance_generator) 
        : bap_algorithms(bap_algorithms), instance_generator(instance_generator) { }

    void set_num_instances(int instances) {
        this->num_of_instances = instances;
        generate_instances(num_of_instances, 0);
    }

    double evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths);
    std::vector<double> calculate_scores(const std::vector<int> &berth_frequencies, 
                                         const std::vector<int> &berth_lengths);
    double aggregate(const std::vector<double> &scores);

    std::vector<double> lower_bounds() { return _lower_bounds; };
    std::vector<double> mwft_not_normalized() { return _mwft_not_normalized; };

private:
    using ScheduleFunction = double (*)(const std::vector<ship> &, const std::vector<berth> &);

    const nlohmann::json &bap_algorithms;
    int num_of_instances = 1;

    std::vector<double> _lower_bounds;
    std::vector<double> _mwft_not_normalized;

    std::vector<std::vector<ship>> instances;
    InstanceGenerator instance_generator;

    ScheduleFunction init_bap(const nlohmann::json &options);
    double calculate_lower_bound(const std::vector<ship> &ships);
    std::vector<berth> berths_to_list(const std::vector<int> &berth_frequencies, 
                                      const std::vector<int> &berth_lengths);

    void generate_instances(int no_instances, int offset);
};

std::vector<ship> read_instance_from_file(int instance_no);
void write_instance_to_file(int instance_no, const std::vector<ship> &ships);

#endif
