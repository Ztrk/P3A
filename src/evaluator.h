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
    Evaluator(const nlohmann::json &bap_algorithms, BaseInstanceGenerator &instance_generator) 
        : bap_algorithms(bap_algorithms), instance_generator(instance_generator) { }

    void set_num_instances(int instances) {
        this->num_of_instances = instances;
        generate_instances(num_of_instances, offset);
    }

    void set_offset(int offset) {
        this->offset = offset;
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
    int offset = 0;

    std::vector<double> _lower_bounds;
    std::vector<double> _mwft_not_normalized;

    std::vector<std::vector<ship>> instances;
    BaseInstanceGenerator &instance_generator;

    ScheduleFunction init_bap(const nlohmann::json &options);
    double calculate_lower_bound(const std::vector<ship> &ships);
    std::vector<berth> berths_to_list(const std::vector<int> &berth_frequencies, 
                                      const std::vector<int> &berth_lengths);

    void generate_instances(int no_instances, int offset);
};

class InstanceGenerator : public BaseInstanceGenerator {
public:
    InstanceGenerator(const std::string &folder, bool instances_from_file)
        : instances_folder(folder), instances_from_file(instances_from_file) { }

    std::vector<ship> generate(int instance_num);
private:
    std::string instances_folder;
    bool instances_from_file;

    std::vector<ship> read_instance_from_file(const std::string &path);
    void write_instance_to_file(const std::string &path, const std::vector<ship> &ships);
};

#endif
