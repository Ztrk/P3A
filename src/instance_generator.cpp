//
//  main.cpp
//  QPP_VesselInstanceGenerator
//

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <math.h>
#include <nlohmann/json.hpp>
//#include <boost/math/distributions/logistic.hpp>
//#include <boost/random/mersenne_twister.hpp>
//#include <boost/random/variate_generator.hpp>
//#include <boost/random/logistic_distribution.hpp>

#include "instance_generator.h"
#include "ship.h"

#define WEEKS_PER_YEAR 52

using namespace std;
using json = nlohmann::json;

namespace generator {
struct options {
    int time_horizon = 1; //-t in years
    int intensity_of_arrivals = 20; //-n per week
    string instance_parameters_file = "lh_instance.json"; //-p
    string output_file = "lh_output.txt"; //-o
    vector<string> clusters = {"cl1", "cl2", "cl3", "cl4", "cl5", "cl6", "cl7"};
} opts;

void display_usage(string name) {
    cerr << "Usage: " << name << " <options>\n"
    << "Options:\n"
    << "\t-h\t\t\tShow this help message\n"
    << endl;
}

void display_ships(vector<ship> &ships) {
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); ++its) {
        cout << "Ship #" << its->no << ": {ready_time: " << its->ready_time << ", length: " << its->length << ", processing_time: " << its->processing_time << ", weight: " << its->weight << "}" << endl;
    }
}

vector<ship> generate_instance(json &instance_parameters) {
    int total_number_of_ships = opts.time_horizon * WEEKS_PER_YEAR * opts.intensity_of_arrivals;
    int max_ready_time = opts.time_horizon * WEEKS_PER_YEAR * 7 * 24;
    int number_of_ships_in_cluster = 0;
    int ship_number = 1;
    vector<ship> ships;
    ship s;
    vector<int> ship_lengths;
    vector<double> gmm_probabilities;
    
    //default_random_engine generator;
    random_device rd;
    mt19937 generator = mt19937(rd());
    
    for(vector<string>::iterator itc = opts.clusters.begin(); itc != opts.clusters.end(); ++itc) {
        
        if(!instance_parameters[*itc].is_null()) {
            number_of_ships_in_cluster = floor(total_number_of_ships * (double) instance_parameters["proportions"][*itc]);
            ship_lengths.clear();
            gmm_probabilities.clear();
            
            for (auto it = instance_parameters[*itc]["Lj"]["set"].begin(); it != instance_parameters[*itc]["Lj"]["set"].end(); ++it) {
                ship_lengths.push_back(it.value());
            }
            
            for(auto it = instance_parameters[*itc]["rj"]["set"].begin(); it != instance_parameters[*itc]["rj"]["set"].end(); ++it) {
                gmm_probabilities.push_back(it.value()["lambda"]);
            }
            
            uniform_int_distribution<int> distribution(0, (int) ship_lengths.size() - 1);
            discrete_distribution<int> distribution_gmm(gmm_probabilities.begin(), gmm_probabilities.end());
            //uniform_int_distribution<int> distribution_rj_start(1, max_ready_time);
            uniform_real_distribution<double> distribution_wj_rnd(0, 1);
            //uniform_int_distribution<int> distribution_wj_rnd(0, 1);
            
            //cout << "MIN: " << distribution_gmm.min() << endl;
            //cout << "MAX: " << distribution_gmm.max() << endl;
            
            while(number_of_ships_in_cluster > 0) {
                
                int length = distribution(generator);
                s.no = ship_number;
                s.length = ship_lengths[length];
                
                //mixture/multimodal distribution: lambda for probability, then mu as mean +- sigma
                //gaussian mixture model
                int gmm_component = distribution_gmm(generator);
                normal_distribution<double> distribution_rj_period(instance_parameters[*itc]["rj"]["set"][gmm_component]["mu"], instance_parameters[*itc]["rj"]["set"][gmm_component]["sigma"]);
                int period;
                while((period = ceil(distribution_rj_period(generator))) < 0);
                
                s.ready_time = 1 + period;//s.ready_time = distribution_rj_start(generator); //s.ready_time = period;
                
                if(instance_parameters[*itc]["pj"]["distribution"] == "lnorm") {
                    lognormal_distribution<double> distribution_pj(instance_parameters[*itc]["pj"]["mean"], instance_parameters[*itc]["pj"]["sd"]);
                    s.processing_time = ceil(distribution_pj(generator) * s.length);
                } else {
                    uniform_real_distribution<double> distribution_pj_prob(0, 1);
                    double prob, logis;
                    while((prob = distribution_pj_prob(generator)) < 0 || (logis = (double) instance_parameters[*itc]["pj"]["location"] + (double) instance_parameters[*itc]["pj"]["scale"] * (double) log(prob / (1 - prob))) < 0);
                    s.processing_time = ceil(s.length * logis);
                    //cout << s.length << " " << (double) instance_parameters[*itc]["pj"]["location"] + (double) instance_parameters[*itc]["pj"]["scale"] * (double) log(prob / (1 - prob)) << endl;
                    //cout << ceil(s.length * logis) << endl;
                    //s.processing_time = ceil(s.length * (double) instance_parameters[*itc]["pj"]["location"] + ((double) instance_parameters[*itc]["pj"]["scale"] * (double) log((double) prob / (1 - prob))));
                    //boost::math::logistic_distribution<double> distribution_pj(instance_parameters["cl1"]["pj"]["location"], instance_parameters["cl1"]["pj"]["scale"]);
                    //boost::variate_generator<boost::mt19937, boost::math::logistic_distribution<double>> generator_pj(boost::mt19937(time(0)), distribution_pj);
                    //s.processing_time = ceil(generator_pj());
                }
                
                s.weight = ceil(s.length * s.processing_time * (0.5 + distribution_wj_rnd(generator))); //our proposition
                
                ships.push_back(s);
                --number_of_ships_in_cluster;
                ++ship_number;
                
                while(distribution_gmm(generator) == gmm_component && number_of_ships_in_cluster > 0 && s.ready_time < max_ready_time) {//while(distribution_gmm(generator) == gmm_component) {
                    //if(number_of_ships_in_cluster > 0) {
                        s.no = ship_number;
                        s.ready_time = (s.ready_time + period <= max_ready_time) ? s.ready_time + period : max_ready_time;
                        ships.push_back(s);
                        --number_of_ships_in_cluster;
                        ++ship_number;
                    //} else
                    //    break;
                }
            }
            
        }

    }
        
    return ships;
}

int main(int argc, const char * argv[]) {
    json instance_parameters;
    ifstream instance_parameters_file(opts.instance_parameters_file);
    instance_parameters_file >> instance_parameters;
    
    //cout << setw(4) << instance_parameters << endl;
    
    vector<ship> ships;
    ships = generate_instance(instance_parameters);
    
    //display_ships(ships);
    
    vector<int> berths;
    
    for (auto it = instance_parameters["berths"].begin(); it != instance_parameters["berths"].end(); ++it) {
        berths.push_back(it.value());
    }
    
    ofstream output_file(opts.output_file);
    
    output_file << ships.size() << endl;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); ++its) {
        output_file << its->no << " " << its->ready_time << " " << its->length << " " << its->processing_time << " " << its->weight << " " << its->owner << endl;
    }
    
    output_file << berths.size() << endl;
    for(vector<int>::iterator itb = berths.begin(); itb != berths.end(); ++itb) {
        output_file << *(itb) << " ";
    }
    
    output_file.close();
    
    return 0;
}
}
