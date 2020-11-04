#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <string>
#include <algorithm>
#include <mpi.h>
//#include "ships.cpp"
#include "ships_algo.cpp"

using namespace std;

class Evaluator
{
private:
    string BAPS[11] = {"fcfs", "wspt", "lsf", "lpt", "lasf", "ssf", "spt", "smsf", "weight_first", "weight_first_spt_later", "spt_first_weight_later"};

    float mwft_from_one_processor = 0;
    int num_of_instances = 1;

public:
    float get_mwft_from_one_processor(){
        return mwft_from_one_processor;
    }

    void set_num_instances(int instances)
    {
        this->num_of_instances = instances;
    }

    void display_usage(char *name, Ships_algo sAlg)
    {
        sAlg.display_usage(name);
    }

    float calculate_lower_bound(Ships_algo &sAlg, vector<ship> &ships_from_instance)
    {
        float lower_bound_n = 0; //nominator
        float lower_bound_d = 0; //denominator
        for (int initial_ship_no = 0; initial_ship_no < ships_from_instance.size(); initial_ship_no++)
        {
            lower_bound_n += ships_from_instance[initial_ship_no].processing_time * ships_from_instance[initial_ship_no].weight;
            lower_bound_d += ships_from_instance[initial_ship_no].weight;
        }
        return lower_bound_n / lower_bound_d;
    }

    void reset_values(Ships_algo &sAlg, vector<ship> &ships_from_instance)
    {
        sAlg.occupied_berths.clear();
        sAlg.global_time = 0;
        //number_of_ships = 0;
        //number_of_berths = 0;
        sAlg.twft = 0; //total weighted flow time
        sAlg.tws = 0;  //total weighted stretch
        sAlg.sum_of_weights = 0;
        sAlg.cmax = 0;
    }

    void restore_ships_from_instance(Ships_algo &sAlg, vector<ship> &ships_from_instance);

    void initialize(int inst_no, Ships_algo &sAlg, vector<ship> &ships_from_instance);

    float schedule(int bap_no, int inst_no, Ships_algo &sAlg, vector<ship> &ships_from_instance);

    float calculateMWFT(int argc, char *argv[])
    {
        float sum = 0;
        int pid, np;
        float pid_mwft_instance_sum = 0;
        MPI_Status status;

        MPI_Init(&argc, &argv);

        MPI_Comm_rank(MPI_COMM_WORLD, &pid);
        MPI_Comm_size(MPI_COMM_WORLD, &np);

        printf("pid: %d\n", pid);

        for (int instance_no = pid + 1; instance_no < this->num_of_instances + 1; instance_no += np)
        {

            float lower_bound;
            float MWFT_BAP;
            vector<bool> baps_to_use = {true, true, true, true, true, true, true, true, true, true, true};
            vector<ship> ships_from_instance;
            float mwft_instance_sum = 0;
            Ships_algo sAlg;

            sAlg.processed_ships.clear();
            initialize(instance_no, sAlg, ships_from_instance);
            mwft_instance_sum = 0;
            for (int bap_schedule = 1; bap_schedule < (sizeof(BAPS) / sizeof(BAPS[0]) + 1); bap_schedule++)
            {
                if (baps_to_use[bap_schedule - 1])
                {

                    reset_values(sAlg, ships_from_instance);
                    opts.scheduling_policy = BAPS[bap_schedule - 1];

                    if (bap_schedule == 1)
                    {
                        lower_bound = calculate_lower_bound(sAlg, ships_from_instance);
                    }

                    MWFT_BAP = schedule(bap_schedule, instance_no, sAlg, ships_from_instance);
                    mwft_instance_sum += (MWFT_BAP / lower_bound);

                    sAlg.processed_ships.clear();
                    //cout << "MWFT: " << MWFT_BAP << " , lower bound: " << lower_bound << " MWFT / LB  : " << MWFT_BAP / lower_bound << endl;
                    //cout << "---------------------------------------\n";
                    //cout << opts.scheduling_policy << cmax << endl;
                }
            }

            pid_mwft_instance_sum += mwft_instance_sum;
            ships_from_instance.clear();
            cout << "proces: " << pid << " sum of MWFT from instance (" << instance_no << ") : " << mwft_instance_sum << endl;
        }
        if (pid == 0)
        {
            float tmp;
            for (int i = 1; i < np; i++)
            {
                MPI_Recv(&tmp, 1, MPI_FLOAT,
                         MPI_ANY_SOURCE, 0,
                         MPI_COMM_WORLD,
                         &status);
                //int sender = status.MPI_SOURCE;

                pid_mwft_instance_sum += tmp;
            }                         
            sum = pid_mwft_instance_sum;
            //cout << "sum: " << sum <<endl;
            MPI_Barrier(MPI_COMM_WORLD);

        }
        else
        {
            MPI_Send(&pid_mwft_instance_sum,
                     1, MPI_FLOAT,
                     0, 0,
                     MPI_COMM_WORLD);

            MPI_Barrier(MPI_COMM_WORLD);
        }
        
        MPI_Finalize();
        return sum;
    }
};

void Evaluator::restore_ships_from_instance(Ships_algo &sAlg, vector<ship> &ships_from_instance)
{
    for (ship ship_data : ships_from_instance)
    {
        /*cout << ship_data.no << " " << ship_data.ready_time << " " << ship_data.length << " " <<
        ship_data.processing_time << " " << ship_data.weight << " " << ship_data.owner << "\n";*/

        ship tmp_ship;
        tmp_ship.no = ship_data.no;
        tmp_ship.ready_time = ship_data.ready_time;
        tmp_ship.length = ship_data.length;
        tmp_ship.processing_time = ship_data.processing_time;
        tmp_ship.weight = ship_data.weight;
        tmp_ship.owner = ship_data.owner;
        sAlg.ships.push_back(tmp_ship);
    }
    //cout<< "ships size() in restore function: " << ships.size() << "\n";
}

void Evaluator::initialize(int inst_no, Ships_algo &sAlg, vector<ship> &ships_from_instance)
{
    sAlg.berths.clear();
    sAlg.ships.clear();
    string no = to_string(inst_no);
    ifstream is("instances/input" + no + ".txt");
    if (is.is_open())
    {
        ship ship;
        berth berth;
        string berths_tmp;

        is >> sAlg.number_of_ships;

        for (int i = 1; i <= sAlg.number_of_ships; i++)
        {
            is >> ship.no >> ship.ready_time >> ship.length >> ship.processing_time >> ship.weight >> ship.owner;
            //preference to big ships
            //ship.weight = ship.processing_time * ship.length; //~throughput
            sAlg.ships.push_back(ship);
            ships_from_instance.push_back(ship);
        }

        is >> sAlg.number_of_berths;

        for (int i = 1; i <= sAlg.number_of_berths; i++)
        {
            berth.no = i;
            is >> berth.length;
            /* -- deprecated -- */
            //if(berth.length > max_ship_length) max_ship_length = berth.length;
            sAlg.berths.push_back(berth);
        }
    }
    else
    {
        cout << "nie wczytuje danych z pliku (" + opts.scheduling_policy + ")\n";
    }
    is.close();
}

float Evaluator::schedule(int open_time, int inst_no,
                          Ships_algo &sAlg, vector<ship> &ships_from_instance)
{
    float mwft = 0;
    float mws = 0;

    timespec elapsedTime;

    if (ships_from_instance.size() == 0)
    {
        //cout << "data must be retrieved from the file\n";
        initialize(inst_no, sAlg, ships_from_instance);
        cout << "ships size (init): " << sAlg.ships.size() << endl;
    }
    else
    {
        restore_ships_from_instance(sAlg, ships_from_instance);
        /*cout << "ships size (copy): " << ships.size() << endl;*/
        //cout << "the data should be stored in a variable called 'ships_from_instance'\n";
    }

    clock_gettime(CLOCK_MONOTONIC, &startProcess);

    if (opts.future_arrivals != 0 && opts.future_arrivals < sAlg.number_of_ships)
        sAlg.list_or_look_ahead_scheduling();
    else
        sAlg.priority_based_scheduling();

    clock_gettime(CLOCK_MONOTONIC, &endProcess);
    elapsedTime = diff(startProcess, endProcess);

    mwft = (1.0 / sAlg.sum_of_weights) * sAlg.twft;
    mws = (1.0 / sAlg.sum_of_weights) * sAlg.tws;

    string out_file = "output/output" + to_string(inst_no) + ".txt";

    //cout << "processed_ship_length: " << processed_ships.size() << endl;
    if (open_time == 0)
    {

        ofstream output_file(out_file);
        output_file << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << sAlg.cmax << " " << mws << endl;
        output_file << sAlg.number_of_berths << " " << sAlg.number_of_ships << endl;
        for (vector<processed_ship>::iterator itps = sAlg.processed_ships.begin(); itps != sAlg.processed_ships.end(); itps++)
            output_file << itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;

        output_file.close();
    }
    else
    {
        ofstream output_file(out_file, std::ios_base::app);
        output_file << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << sAlg.cmax << " " << mws << endl;
        output_file << sAlg.number_of_berths << " " << sAlg.number_of_ships << endl;
        for (vector<processed_ship>::iterator itps = sAlg.processed_ships.begin(); itps != sAlg.processed_ships.end(); itps++)
            output_file << itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;

        output_file.close();
    }

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