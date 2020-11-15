#include "evaluator.h"
#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "greedy.cpp"
#include "instance_generator.h"

using namespace std;

void Evaluator::restore_ships_from_instance() {
    for (generator::ship ship_data : ships_from_instance) {
        /*cout << ship_data.no << " " << ship_data.ready_time << " " << ship_data.length << " " <<
        ship_data.processing_time << " " << ship_data.weight << " " << ship_data.owner << "\n";*/

        ship tmp_ship;
        tmp_ship.no = ship_data.no;
        tmp_ship.ready_time  = ship_data.ready_time;
        tmp_ship.length = ship_data.length;
        tmp_ship.processing_time = ship_data.processing_time;
        tmp_ship.weight= ship_data.weight;
        tmp_ship.owner = ship_data.owner;
        ships.push_back(tmp_ship);
    }
    //cout<< "ships size() in restore function: " << ships.size() << "\n";
}

void Evaluator::initialize()
{
    berths.clear();
    ships.clear();

    ships_from_instance = generator::generate_instance();

    restore_ships_from_instance();

    int berth_no = 0;
    for (size_t i = 0; i < berth_frequencies.size(); ++i) {
        for (int j = 0; j < berth_frequencies[i]; ++j) {
            berth berth;
            berth.no = berth_no++;
            berth.length = berth_lengths[i];
            berths.push_back(berth);
        }
    }
}

float Evaluator::schedule()
{
    float mwft = 0;
    float mws = 0;

    timespec elapsedTime;

    if(ships_from_instance.size() == 0) {
        //cout << "data must be retrieved from the file\n";
        initialize();
        cout << "ships size (init): " << ships.size() << endl;
    } else {
        restore_ships_from_instance();
        /*cout << "ships size (copy): " << ships.size() << endl;*/
        //cout << "the data should be stored in a variable called 'ships_from_instance'\n";
    }

    clock_gettime(CLOCK_MONOTONIC, &startProcess);

    if(opts.future_arrivals != 0 && opts.future_arrivals < number_of_ships)
        list_or_look_ahead_scheduling();
    else
        priority_based_scheduling();

    clock_gettime(CLOCK_MONOTONIC, &endProcess);
    elapsedTime = diff(startProcess, endProcess);

    mwft = (1.0 / sum_of_weights) * twft;
    mws = (1.0 / sum_of_weights) * tws;


    /*
    string out_file = "output/output" + to_string(inst_no) + ".txt";

    //cout << "processed_ship_length: " << processed_ships.size() << endl;
    if(open_time == 0) {

        ofstream output_file(out_file);
        output_file << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << cmax << " " << mws << endl;
        output_file << number_of_berths << " " << number_of_ships << endl;
        for(vector<processed_ship>::iterator itps = processed_ships.begin(); itps != processed_ships.end(); itps++)
            output_file << itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;

        output_file.close();
    } else {
        ofstream output_file(out_file, std::ios_base::app);
        output_file << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << cmax << " " << mws << endl;
        output_file << number_of_berths << " " << number_of_ships << endl;
        for(vector<processed_ship>::iterator itps = processed_ships.begin(); itps != processed_ships.end(); itps++)
            output_file << itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;

        output_file.close();
    }
    */


    /*if(open_time == 0) {
        ofstream output_file_with_def("out_def.txt");
        output_file_with_def << "TIME(ms) " << "MWFT " << "CMAX " << "MWS " << endl;
        output_file_with_def << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << cmax << " " << mws << endl;
        output_file_with_def << "berths no: " << number_of_berths << " ships no:" << number_of_ships << endl;
        for(vector<processed_ship>::iterator itps = processed_ships.begin(); itps != processed_ships.end(); itps++)
            output_file_with_def << "processed_ship.no: " << itps->no << " , start_time: " << itps->start_time << " berth_no: " << itps->berth_no << " processing_time: " << itps->processing_time << " flow_time: " << itps->flow_time << endl;
        output_file_with_def << "\n";

        output_file_with_def.close();
    } else {
        ofstream output_file_with_def("out_def.txt", std::ios_base::app);
        output_file_with_def << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << cmax << " " << mws << endl;
        output_file_with_def << number_of_berths << " " << number_of_ships << endl;
        for(vector<processed_ship>::iterator itps = processed_ships.begin(); itps != processed_ships.end(); itps++)
            output_file_with_def<< itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;
        output_file_with_def << "\n";

        output_file_with_def.close();
    }*/
    //z definicjami


    /*output_file_with_def << "TIME(MS): " << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << "\nMWFT: " << mwft << "\nCMAX: " << cmax << "\nMWS: " << mws << endl;
    output_file_with_def << "berths no: " << number_of_berths << " ships no:" << number_of_ships << endl;
    for(vector<processed_ship>::iterator itps = processed_ships.begin(); itps != processed_ships.end(); itps++)
        output_file_with_def << "processed_ship.no: " << itps->no << " , start_time: " << itps->start_time << " berth_no: " << itps->berth_no << " processing_time: " << itps->processing_time << " flow_time: " << itps->flow_time << endl;
    output_file_with_def << "\n";*/
    return mwft;
}

float Evaluator::calculateMWFT() {
    double mwft_from_one_processor = 0;
    for (int instance_no = 1; instance_no < this->num_of_instances+ 1; instance_no++) {
        processed_ships.clear();
        initialize();
        mwft_instance_sum = 0;
        lower_bound = calculate_lower_bound();
        for (size_t i = 0; i < bap_algorithms.size(); ++i) {
            init_bap(bap_algorithms[i]);
            reset_values();
            //opts.scheduling_policy = BAPS[bap_schedule - 1];

            MWFT_BAP = schedule();
            mwft_instance_sum += (MWFT_BAP / lower_bound);

            processed_ships.clear();
        }

        mwft_from_one_processor += mwft_instance_sum;
        ships_from_instance.clear();
        cout << "sum of MWFT from instance (" << instance_no << ") : " << mwft_instance_sum << endl;
    }
    return mwft_from_one_processor;
}

void Evaluator::reset_values() {
    occupied_berths.clear();
    global_time = 0;
    //number_of_ships = 0;
    //number_of_berths = 0;
    twft = 0; //total weighted flow time
    tws = 0; //total weighted stretch
    sum_of_weights = 0;
    cmax = 0;
}

float Evaluator::calculate_lower_bound() {
    float lower_bound_n = 0; //nominator
    float lower_bound_d = 0; //denominator
    for (size_t initial_ship_no = 0; initial_ship_no < ships_from_instance.size(); initial_ship_no++) {
        lower_bound_n += ships_from_instance[ initial_ship_no ].processing_time * ships_from_instance[ initial_ship_no ].weight;
        lower_bound_d += ships_from_instance[ initial_ship_no ].weight;

    }
    return lower_bound_n / lower_bound_d;
}

double Evaluator::evaluate(const std::vector<int> &berth_frequencies, const std::vector<int> &berth_lengths) {
    this->berth_frequencies = berth_frequencies;
    this->berth_lengths = berth_lengths;

    double mwft = calculateMWFT();
    cout << "total mwft on processor: " << mwft << endl;

    return mwft;
}

void Evaluator::init_bap(const nlohmann::json &options) {
    if (options["algorithm"].get<string>() == "greedy") {
        opts.scheduling_policy = options["scheduling_policy"].get<string>();
        opts.future_arrivals = options["future_arrivals"].get<int>();
    }
    else {
        throw invalid_argument("Wrong value in config file: 'algorithm' should be one of: 'greedy'");
    }
}
