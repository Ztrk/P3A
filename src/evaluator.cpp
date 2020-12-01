#include "evaluator.h"
#include <iostream>
#include <fstream>
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
    _lower_bounds.clear();
    _mwft_not_normalized.clear();

    for (int instance_no = 0; instance_no < this->num_of_instances; instance_no++) {
        vector<ship> &ships = instances[instance_no];
        double lower_bound = calculate_lower_bound(ships);

        for (size_t i = 0; i < bap_algorithms.size(); ++i) {
            ScheduleFunction schedule = init_bap(bap_algorithms[i]);

            double mwft = schedule(ships, berths);

            _lower_bounds.push_back(lower_bound);
            _mwft_not_normalized.push_back(mwft);

            double score = mwft / lower_bound;
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

void Evaluator::generate_instances(int no_instances, int offset) {
    instances = vector<vector<ship>>(no_instances);
    for (int i = 0; i < no_instances; ++i) {
        instances[i] = instance_generator.generate(i + offset);
    }
}

std::vector<ship> InstanceGenerator::generate(int instance_num) {
    vector<ship> ships;
    string number = to_string(instance_num);
    if (instances_from_file) {
        ships = read_instance_from_file(instances_folder + "instance" + number + ".txt");
    }
    else {
        ships = generator::generate_instance();
    }
    write_instance_to_file(instances_folder + "used-instance" + number + ".txt", ships);
    return ships;
}

vector<ship> InstanceGenerator::read_instance_from_file(const string &path) {
    ifstream is(path);
    if (is.fail()) {
        throw runtime_error("Could not open instance file " + path);
    }

    int n;
    is >> n;

    vector<ship> ships(n);
    for (int i = 0; i < n; i++) {
        is  >> ships[i].no >> ships[i].ready_time >> ships[i].length 
            >> ships[i].processing_time >> ships[i].weight >> ships[i].owner;
    }
    return ships;
}

void InstanceGenerator::write_instance_to_file(const string &path, const vector<ship> &ships) {
    ofstream file(path);
    if (file.fail()) {
        // throw runtime_error("Could not save instance in file " + path);
        cerr << "Could not save instance in file " + path << endl;
        return;
    }

    file << ships.size() << '\n';
    for (size_t i = 0; i < ships.size(); i++) {
        file << ships[i].no << ' ' << ships[i].ready_time << ' ' << ships[i].length << ' '
             << ships[i].processing_time << ' ' << ships[i].weight << ' ' << ships[i].owner << '\n';
    }
}
