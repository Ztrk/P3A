#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <vector>
#include <string>
#include "evaluator_interface.h"
#include "instance_generator.h"

class Evaluator : public EvaluatorInterface {
private:
    std::vector<std::string> BAPS = {"fcfs", "wspt", "lsf", "lpt", "lasf", "ssf", "spt", "smsf", "weight_first", "weight_first_spt_later", "spt_first_weight_later"};
    float lower_bound;
    std::vector<bool> baps_to_use = {true, true, true, true, true, true, true, true, true, true, true};
    std::vector<generator::ship> ships_from_instance;
    float MWFT_BAP;
    float mwft_instance_sum = 0;
    float mwft_from_one_processor = 0;
    int num_of_instances = 1;

    std::vector<int> berth_frequencies;
    std::vector<int> berth_lengths;
public:
    void set_num_instances(int instances) {
        this->num_of_instances = instances;
    }

    float calculate_lower_bound();

    void reset_values();

    void restore_ships_from_instance();

    void initialize();

    float schedule(int bap_no, int inst_no);

    float calculateMWFT();

    double evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths);
};

#endif
