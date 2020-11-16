#include "evaluator.h"
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "ship.h"
#include "greedy.h"
#include "ils.h"
#include "instance_generator.h"

using namespace std;

vector<berth> Evaluator::berths_to_list(const std::vector<int> &berth_frequencies, 
                                        const std::vector<int> &berth_lengths) {
    vector<berth> berths;
    int berth_no = 0;
    for (size_t i = 0; i < berth_frequencies.size(); ++i) {
        for (int j = 0; j < berth_frequencies[i]; ++j) {
            berth berth;
            berth.no = berth_no++;
            berth.length = berth_lengths[i];
            berths.push_back(berth);
        }
    }
    return berths;
}

vector<double> Evaluator::calculate_scores(const std::vector<int> &berth_frequencies, 
                                           const std::vector<int> &berth_lengths) 
{
    vector<berth> berths = berths_to_list(berth_frequencies, berth_lengths);
    vector<double> scores;

    for (int instance_no = 0; instance_no < this->num_of_instances; instance_no++) {
        vector<ship> ships = generator::generate_instance();
        double lower_bound = calculate_lower_bound(ships);

        for (size_t i = 0; i < bap_algorithms.size(); ++i) {
            ScheduleFunction schedule = init_bap(bap_algorithms[i]);

            double mwft = schedule(ships, berths);
            double score = mwft / lower_bound;
            // cout << score << ' ' << mwft << ' ' << lower_bound << endl;
            scores.push_back(score);
        }
    }
    return scores;
}

double Evaluator::calculate_lower_bound(const vector<ship> &ships) {
    double lower_bound_n = 0; //nominator
    double lower_bound_d = 0; //denominator
    for (size_t i = 0; i < ships.size(); ++i) {
        lower_bound_n += ships[i].processing_time * ships[i].weight;
        lower_bound_d += ships[i].weight;
    }
    return lower_bound_n / lower_bound_d;
}

double Evaluator::evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths) {
    vector<double> scores = calculate_scores(berth_frequencies, berth_lengths);
    return aggregate(scores);
}

Evaluator::ScheduleFunction Evaluator::init_bap(const nlohmann::json &options) {
    string algorithm = options["algorithm"].get<string>();
    if (algorithm == "greedy") {
        greedy::opts.scheduling_policy = options["scheduling_policy"].get<string>();
        greedy::opts.future_arrivals = options["future_arrivals"].get<int>();
        return greedy::schedule;
    }
    else if (algorithm == "ils") {
        ils::opts.destruction_method = options.value("destruction_method", "C");
        ils::opts.destruction_fraction = options.value("destruction_fraction", 0.1);
        return ils::schedule;
    }
    else {
        throw invalid_argument("Wrong value in config file: 'algorithm' should be one of: 'greedy', 'ils'");
    }
}

double Evaluator::aggregate(const std::vector<double> &scores) {
    double sum = 0.0;
    for (size_t i = 0; i < scores.size(); ++i) {
        sum += scores[i];
    }
    return sum / scores.size();
}
