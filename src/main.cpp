#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <ctime>
#include <string>

using namespace std;

struct options
{
    string input_file; //-i
    string output_file; //-o
    string scheduling_policy; //-p
    int future_arrivals; //-k
} opts;

struct ship
{
    int no; //# of ship
    int ready_time; //ready time
    int length; //length
    int processing_time; //processing time
    int weight; //weight
    int owner; //owner
};

struct berth
{
    int no; //# of berth
    int length; //length
};

struct processed_ship
{
    int no;
    int start_time;
    int berth_no;
    // -- histogram of flow time --
    int processing_time;
    int flow_time; //flow time of given ship
};

struct occupied_berth
{
    int no;
    int occupied_length;
    int release_time;
    int ship_no;
};

int global_time = 0;
int number_of_ships = 0;
int number_of_berths = 0;
float twft = 0; //total weighted flow time
float tws = 0; //total weighted stretch
int sum_of_weights = 0;
int cmax = 0;
vector<ship> ships;
vector<berth> berths;
vector<processed_ship> processed_ships;
vector<occupied_berth> occupied_berths;

timespec startProcess, endProcess;

timespec diff(timespec start, timespec end);

double to_milliseconds(timespec t);

unsigned long long to_nanoseconds(timespec t);

/* -- deprecated -- */
//int max_ship_length = 0;
class Evaluator {
private:
    string BAPS[11] = {"fcfs", "wspt", "lsf", "lpt", "lasf", "ssf", "spt", "smsf", "weight_first", "weight_first_spt_later", "spt_first_weight_later"};
    float lower_bound;
    vector<bool> baps_to_use = {true, true, true, true, true, true, true, true, true, true, true};
    vector<ship> ships_from_instance;
    float MWFT_BAP;
    float mwft_instance_sum = 0;
    float mwft_from_one_processor = 0;
    int num_of_instances = 1;
public:
    void set_num_instances(int instances) {
        this->num_of_instances = instances;
    }

    float calculate_lower_bound() {
        float lower_bound_n = 0; //nominator
        float lower_bound_d = 0; //denominator
        for(int initial_ship_no = 0; initial_ship_no < ships_from_instance.size(); initial_ship_no++) {
            lower_bound_n += ships_from_instance[ initial_ship_no ].processing_time * ships_from_instance[ initial_ship_no ].weight;
            lower_bound_d += ships_from_instance[ initial_ship_no ].weight;

        }
        return lower_bound_n / lower_bound_d;
    }

    void reset_values() {
        occupied_berths.clear();
        global_time = 0;
        //number_of_ships = 0;
        //number_of_berths = 0;
        twft = 0; //total weighted flow time
        tws = 0; //total weighted stretch
        sum_of_weights = 0;
        cmax = 0;
    }

    void restore_ships_from_instance();

    void initialize(int inst_no);

    float schedule(int bap_no, int inst_no);

    float calculateMWFT() {
        for(int instance_no = 1; instance_no < this->num_of_instances+ 1; instance_no++) {

            processed_ships.clear();
            initialize(instance_no);
            mwft_instance_sum = 0;
            for(int bap_schedule = 1; bap_schedule < ( sizeof(BAPS)/sizeof(BAPS[0]) + 1); bap_schedule++) {
                if(baps_to_use[ bap_schedule - 1 ]) {

                    reset_values();
                    opts.scheduling_policy = BAPS[bap_schedule - 1];

                    if(bap_schedule == 1)
                    {
                        lower_bound = calculate_lower_bound();
                    }

                    MWFT_BAP = schedule(bap_schedule, instance_no);
                    mwft_instance_sum += (MWFT_BAP / lower_bound);

                    processed_ships.clear();
                    /*cout << "MWFT: " << MWFT_BAP << " , lower bound: " << lower_bound << " MWFT / LB  : " << MWFT_BAP / lower_bound << endl;*/
                    //cout << "---------------------------------------\n";
                    //cout << opts.scheduling_policy << cmax << endl;
                }
            }

            mwft_from_one_processor += mwft_instance_sum;
            ships_from_instance.clear();
            cout << "sum of MWFT from instance (" << instance_no << ") : " << mwft_instance_sum << endl;
        }
        return mwft_from_one_processor;
    }
};

void Evaluator::restore_ships_from_instance() {
    for(ship ship_data : ships_from_instance) {
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

void display_usage(string name)
{
    cerr << "Usage: " << name << " <options>\n"
         << "Options:\n"
         << "\t-h\t\tShow this help message\n"
         << "\t-i FILE\t\tSpecify the input file\n"
         << "\t-o FILE\t\tSpecify the output file\n"
         << "\t-p POLICY\tSpecify the scheduling policy (algorithm)\n\t\t\t[fcfs, wspt, lsf, lpt, lasf, ssf, spt, smsf, random_heuristic, weight_first,\t\t\tweight_first_spt_later, spt_first_weight_later]\n"
         << "\t-k NUMBER\tSpecify the number of future arrivals to check\n"
         << endl;
}

void Evaluator::initialize(int inst_no)
{
    berths.clear();
    ships.clear();
    string no = to_string(inst_no);
    ifstream is("input" + no + ".txt");
    if(is.is_open())
    {
        ship ship;
        berth berth;
        string berths_tmp;

        is >> number_of_ships;

        for(int i = 1; i <= number_of_ships; i++)
        {
            is >> ship.no >> ship.ready_time >> ship.length >> ship.processing_time >> ship.weight >> ship.owner;
            //preference to big ships
            //ship.weight = ship.processing_time * ship.length; //~throughput
            ships.push_back(ship);
            ships_from_instance.push_back(ship);
        }

        is >> number_of_berths;

        for(int i = 1; i <= number_of_berths; i++)
        {
            berth.no = i;
            is >> berth.length;
            /* -- deprecated -- */
            //if(berth.length > max_ship_length) max_ship_length = berth.length;
            berths.push_back(berth);
        }
    } else {
        cout << "nie wczytuje danych z pliku (" + opts.scheduling_policy + ")\n";
    }
    is.close();
}

void display_data()
{
    cout << number_of_ships << endl;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); ++its)
        cout << its->no << " " << its->ready_time << " " << its->length << " " << its->processing_time << " " << its->weight << " " << its->owner << endl;
    cout << number_of_berths << endl;
    for(vector<berth>::iterator itb = berths.begin(); itb != berths.end(); ++itb)
        cout << itb->no << " " << itb->length << endl;
}

int random_heuristic()
{
    random_shuffle(ships.begin(), ships.end());
    return 0;
}

int random_heuristic(vector<ship> &available_ships)
{
    random_shuffle(available_ships.begin(), available_ships.end());
    return 0;
}

int fcfs()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->ready_time < ships.at(ship_index).ready_time) ship_index = its - ships.begin();
    return ship_index;
}

int fcfs(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->ready_time < available_ships.at(ship_index).ready_time) ship_index = itas - available_ships.begin();
    return ship_index;
}

int wspt()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->processing_time/its->weight < ships.at(ship_index).processing_time/ships.at(ship_index).weight) ship_index = its - ships.begin();
    return ship_index;
}

int wspt(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->processing_time/itas->weight < available_ships.at(ship_index).processing_time/available_ships.at(ship_index).weight) ship_index = itas - available_ships.begin();
    return ship_index;
}

int lsf()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->length > ships.at(ship_index).length) ship_index = its - ships.begin();
    return ship_index;
}

int lsf(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->length > available_ships.at(ship_index).length) ship_index = itas - available_ships.begin();
    return ship_index;
}

int lpt()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->processing_time > ships.at(ship_index).processing_time) ship_index = its - ships.begin();
    return ship_index;
}

int lpt(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->processing_time > available_ships.at(ship_index).processing_time) ship_index = itas - available_ships.begin();
    return ship_index;
}

int lasf()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->processing_time * its->length > ships.at(ship_index).processing_time * ships.at(ship_index).length) ship_index = its - ships.begin();
    return ship_index;
}

int lasf(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->processing_time * itas->length > available_ships.at(ship_index).processing_time * available_ships.at(ship_index).length) ship_index = itas - available_ships.begin();
    return ship_index;
}

int ssf()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->length < ships.at(ship_index).length) ship_index = its - ships.begin();
    return ship_index;
}

int ssf(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->length < available_ships.at(ship_index).length) ship_index = itas - available_ships.begin();
    return ship_index;
}

int spt()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->processing_time < ships.at(ship_index).processing_time) ship_index = its - ships.begin();
    return ship_index;
}

int spt(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->processing_time < available_ships.at(ship_index).processing_time) ship_index = itas - available_ships.begin();
    return ship_index;
}

int smsf()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->processing_time * its->length < ships.at(ship_index).processing_time * ships.at(ship_index).length) ship_index = its - ships.begin();
    return ship_index;
}

int smsf(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->processing_time * itas->length < available_ships.at(ship_index).processing_time * available_ships.at(ship_index).length) ship_index = itas - available_ships.begin();
    return ship_index;
}

int weight_first() // weight first
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->weight > ships.at(ship_index).weight) ship_index = its - ships.begin();
    return ship_index;
}

int weight_first(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->weight > available_ships.at(ship_index).weight) ship_index = itas - available_ships.begin();
    return ship_index;
}

int weight_first_spt_later()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->weight >= ships.at(ship_index).weight)
        {
            if(its->weight == ships.at(ship_index).weight)
            {
                if(its->processing_time < ships.at(ship_index).processing_time) ship_index = its - ships.begin();
                // do nothing, i.e. ship at given ship_index is `better`
            }
            else
                ship_index = its - ships.begin();
        }
    return ship_index;
}

int weight_first_spt_later(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->weight >= available_ships.at(ship_index).weight)
        {
            if(itas->weight == available_ships.at(ship_index).weight)
            {
                if(itas->processing_time < available_ships.at(ship_index).processing_time) ship_index = itas - available_ships.begin();
                // do nothing, i.e. ship at given ship_index is `better`
            }
            else
                ship_index = itas - available_ships.begin();
        }
    return ship_index;
}

int spt_first_weight_later()
{
    int ship_index = 0;
    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->processing_time <= ships.at(ship_index).processing_time)
        {
            if(its->processing_time == ships.at(ship_index).processing_time)
            {
                if(its->weight > ships.at(ship_index).weight) ship_index = its - ships.begin();
                // do nothing, i.e. ship at given ship_index is `better`
            }
            else
                ship_index = its - ships.begin();
        }
    return ship_index;
}

int spt_first_weight_later(vector<ship> &available_ships)
{
    int ship_index = 0;
    for(vector<ship>::iterator itas = available_ships.begin(); itas != available_ships.end(); itas++)
        if(itas->processing_time <= available_ships.at(ship_index).processing_time)
        {
            if(itas->processing_time == available_ships.at(ship_index).processing_time)
            {
                if(itas->weight > available_ships.at(ship_index).weight) ship_index = itas - available_ships.begin();
                // do nothing, i.e. ship at given ship_index is `better`
            }
            else
                ship_index = itas - available_ships.begin();
        }
    return ship_index;
}

int next_ship_index()
{
    if(opts.scheduling_policy == "fcfs") return fcfs();
    if(opts.scheduling_policy == "wspt") return wspt();
    if(opts.scheduling_policy == "lsf") return lsf();
    if(opts.scheduling_policy == "lpt") return lpt();
    if(opts.scheduling_policy == "lasf") return lasf();
    if(opts.scheduling_policy == "ssf") return ssf();
    if(opts.scheduling_policy == "spt") return spt();
    if(opts.scheduling_policy == "smsf") return smsf();
    if(opts.scheduling_policy == "weight_first") return weight_first();
    if(opts.scheduling_policy == "weight_first_spt_later") return weight_first_spt_later();
    if(opts.scheduling_policy == "spt_first_weight_later") return spt_first_weight_later();
    return random_heuristic();
}

int next_ship_index(vector<ship> &available_ships)
{
    //if(available_ships.size() == 0) return -1;
    if(opts.scheduling_policy == "fcfs") return fcfs(available_ships);
    if(opts.scheduling_policy == "wspt") return wspt(available_ships);
    if(opts.scheduling_policy == "lsf") return lsf(available_ships);
    if(opts.scheduling_policy == "lpt") return lpt(available_ships);
    if(opts.scheduling_policy == "lasf") return lasf(available_ships);
    if(opts.scheduling_policy == "ssf") return ssf(available_ships);
    if(opts.scheduling_policy == "spt") return spt(available_ships);
    if(opts.scheduling_policy == "smsf") return smsf(available_ships);
    if(opts.scheduling_policy == "weight_first") return weight_first(available_ships);
    if(opts.scheduling_policy == "weight_first_spt_later") return weight_first_spt_later(available_ships);
    if(opts.scheduling_policy == "spt_first_weight_later") return spt_first_weight_later(available_ships);
    return random_heuristic(available_ships);
}

int next_berth_index(ship &ship)
{
    int berth_index = -1;

    for(vector<berth>::iterator itb = berths.begin(); itb != berths.end(); itb++)
    {
        int occupied_length = 0;
        int number_of_handled_ships = 0;
        for(vector<occupied_berth>::iterator itob = occupied_berths.begin(); itob != occupied_berths.end(); itob++)
        {
            if(itb->no == itob->no)
            {
                occupied_length += itob->occupied_length;
                ++number_of_handled_ships;
            }
        }

        if(number_of_handled_ships < 2 && ship.length <= itb->length - occupied_length)
        {
            if(berth_index == -1)
                berth_index = itb - berths.begin();
            else // 28112016: nowy sposób wyboru nabrzeża --- ! ----
            if(itb->length - occupied_length - ship.length < berths.at(berth_index).length - occupied_length - ship.length) berth_index = itb - berths.begin();//if(itb->length - occupied_length < berths.at(berth_index).length) berth_index = itb - berths.begin();//!!!!!
        }
    }

    return berth_index;
}

bool release_berths()
{
    bool berth_released = false;
    vector<occupied_berth>::iterator itob = occupied_berths.begin();

    while(itob != occupied_berths.end())
    {
        if(itob->release_time <= global_time)
        {
            itob = occupied_berths.erase(itob);
            berth_released = true;
        }
        else
            ++itob;
    }

    return berth_released;
}

bool check_ship_availability()
{
    bool available = false;

    for(vector<ship>::iterator its = ships.begin(); its != ships.end(); its++)
        if(its->ready_time <= global_time) available = true;

    return available;
}

void set_available_ships(vector<ship> &available_ships)
{
    vector<ship>::iterator its = ships.begin();

    while(its != ships.end())
    {
        int k = 0;
        if(its->ready_time <= global_time)
        {
            available_ships.push_back(ships.at(its - ships.begin()));
            its = ships.erase(its);
            k++;
            int future_time = 0;
            while(k < opts.future_arrivals)
            {
                its = ships.begin();
                //vector<ship>::iterator itk = ships.begin();
                while(its != ships.end())
                {
                    if(k < opts.future_arrivals)
                    {
                        if(its->ready_time <= global_time + future_time)
                        {
                            available_ships.push_back(ships.at(its - ships.begin()));
                            its = ships.erase(its);
                            k++;
                        }
                        else
                            ++its;
                    }
                    else
                        break;
                }
                if(ships.size() == 0) break;
                future_time++;
                //its = ships.begin();
            }
        }
        else
            ++its;
    }
}

void ship_processing()
{
    int ship_index = -1;
    int berth_index = -1;

    ship_index = next_ship_index();
    ship ship = ships.at(ship_index);

    /* -- deprecated -- */
    /*if(ship.length > max_ship_length) //skip
    {
        processed_ship processed_ship;
        processed_ship.no = ship.no;
        processed_ship.start_time = -1;
        processed_ship.berth_no = -1;
        processed_ships.push_back(processed_ship);
        ships.erase(ships.begin() + ship_index);
        return;//continue;
    }*/

    while(ship.ready_time > global_time)
        global_time++;
    release_berths();

    berth_index = next_berth_index(ship);

    if(berth_index != -1)
    {
        processed_ship processed_ship;
        occupied_berth occupied_berth;
        berth berth = berths.at(berth_index);

        processed_ship.no = ship.no;
        processed_ship.start_time = global_time;
        processed_ship.berth_no = berth.no;

        occupied_berth.no = berth.no;
        occupied_berth.occupied_length = ship.length;
        occupied_berth.release_time = global_time + ship.processing_time;
        occupied_berth.ship_no = ship.no;

        sum_of_weights += ship.weight;
        twft += (global_time + ship.processing_time - ship.ready_time) * ship.weight;
        tws += ((global_time + ship.processing_time - ship.ready_time) / ship.processing_time) * ship.weight;
        cmax = global_time + ship.processing_time;

        processed_ship.processing_time = ship.processing_time;
        processed_ship.flow_time = global_time + ship.processing_time - ship.ready_time;

        processed_ships.push_back(processed_ship);
        occupied_berths.push_back(occupied_berth);
        ships.erase(ships.begin() + ship_index);
    }
    else
        global_time++;
}

void ship_processing(vector<ship> &available_ships)
{
    while(!available_ships.empty())
    {
        int ship_index = -1;
        int berth_index = -1;

        ship_index = next_ship_index(available_ships);
        //if(ship_index == -1) return false;
        ship ship = available_ships.at(ship_index);

        /* -- deprecated -- */
        /*if(ship.length > max_ship_length) //skip
        {
            processed_ship processed_ship;
            processed_ship.no = ship.no;
            processed_ship.start_time = -1;
            processed_ship.berth_no = -1;
            processed_ships.push_back(processed_ship);
            available_ships.erase(available_ships.begin() + ship_index);
            continue;
        }*/

        while(ship.ready_time > global_time)
            global_time++;
        release_berths();

        berth_index = next_berth_index(ship);

        if(berth_index != -1)
        {
            processed_ship processed_ship;
            occupied_berth occupied_berth;
            berth berth = berths.at(berth_index);

            processed_ship.no = ship.no;
            processed_ship.start_time = global_time;
            processed_ship.berth_no = berth.no;

            occupied_berth.no = berth.no;
            occupied_berth.occupied_length = ship.length;
            occupied_berth.release_time = global_time + ship.processing_time;
            occupied_berth.ship_no = ship.no;

            sum_of_weights += ship.weight;
            twft += (global_time + ship.processing_time - ship.ready_time) * ship.weight;
            tws += ((global_time + ship.processing_time - ship.ready_time) / ship.processing_time) * ship.weight;
            cmax = global_time + ship.processing_time;

            processed_ship.processing_time = ship.processing_time;
            processed_ship.flow_time = global_time + ship.processing_time - ship.ready_time;

            processed_ships.push_back(processed_ship);
            occupied_berths.push_back(occupied_berth);
            available_ships.erase(available_ships.begin() + ship_index);
        }
        else
            break; //no other possible assignment
    }
}

void list_or_look_ahead_scheduling()
{
    vector<ship> available_ships;

    while(!ships.empty() || !available_ships.empty())
    {
        if(release_berths())
        {
            set_available_ships(available_ships);
            ship_processing(available_ships);
        }

        set_available_ships(available_ships);
        ship_processing(available_ships);

        if(!check_ship_availability()) global_time++; //ships with ready_time less than global_time

        //global_time++;
        //if(!ships.empty() && available_ships.empty()) global_time++;
    }
}

void priority_based_scheduling()
{
    while(!ships.empty())
    {
        release_berths();
        ship_processing();
    }
}

float Evaluator::schedule(int open_time, int inst_no)
{
    float mwft = 0;
    float mws = 0;

    timespec elapsedTime;

    if(ships_from_instance.size() == 0) {
        //cout << "data must be retrieved from the file\n";
        initialize(inst_no);
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


    string out_file = "output" + to_string(inst_no) + ".txt";

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

int main(int argc, char *argv[])
{
    srand(time(0)); //random seed

    opts.input_file = "input2.txt";
    opts.output_file = "out2.txt";
    /*opts.input_file = "";
    opts.output_file = "";*/
    opts.scheduling_policy = "";
    opts.future_arrivals = 0;

    int opt;

    while((opt = getopt(argc, argv, "i:o:p:k:h?")) != -1)
    {
        switch(opt)
        {
            case 'i':
                opts.input_file = optarg;
                break;

            case 'o':
                opts.output_file = optarg;
                break;

            case 'p':
                opts.scheduling_policy = optarg;
                break;

            case 'k':
                opts.future_arrivals = atoi(optarg);
                break;

            case 'h':
            case '?':
            default:
                display_usage(argv[0]);
        }
    }

    if(opts.input_file == "")
    {
        cerr << "Error: input file ('-i') not specified!" << endl;
        return EXIT_FAILURE;
    }

    if(opts.output_file == "")
    {
        cerr << "Error: output file ('-o') not specified!" << endl;
        return EXIT_FAILURE;
    }

    /*if(opts.scheduling_policy == "")
    {
        cerr << "Error: scheduling policy ('-p') not specified!" << endl;
        return EXIT_FAILURE;
    }*/

    Evaluator simple_eval;

    simple_eval.set_num_instances(8);

    float sum_mwft_proc = simple_eval.calculateMWFT();

    cout << "total mwft on processor: " << sum_mwft_proc << endl;




    //schedule();

    return EXIT_SUCCESS;
}

timespec diff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

double to_milliseconds(timespec t)
{
    double temp;

    temp = (double) t.tv_sec * 1000.0;
    temp += (double) t.tv_nsec / 1000000.0;

    return temp;
}

unsigned long long to_nanoseconds(timespec t)
{
    unsigned long long temp;

    temp = t.tv_sec * 1000000000;
    temp += t.tv_nsec;

    return temp;
}