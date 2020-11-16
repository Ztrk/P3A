#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <array>
#include <map>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
//#include <windows.h>
#include <time.h>

#include "ils.h"
#include "ship.h"

#define TIME_LIMIT 10

using namespace std;

namespace ils {
options opts;

struct processed_ship
{
	int no;
	int start_time;
	//int berth_no;
	// -- histogram of flow time --
	int ready_time;
	int length;
	int processing_time;
	int flow_time; //flow time of given ship
	int weight;
};

struct occupied_berth
{
	int no;
	int occupied_length;
	int occupied_side;
	int release_time;
	int ship_no;
};

struct processed_berth
{
	int no;
	int length;
	vector<processed_ship> left;
	vector<processed_ship> right;
};

struct quality_log
{
	timespec actual_time;
	float mwft;
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
vector<processed_berth> processed_berths;
vector<occupied_berth> occupied_berths;
vector<string> scheduling_policies = {"fcfs", "wspt", "lsf", "lpt", "lasf", "ssf", "spt", "smsf", "random_heuristic", "weight_first", "weight_first_spt_later", "spt_first_weight_later"};
vector<vector<processed_berth>> initial_solutions;
vector<vector<processed_berth>> final_solutions;
vector<quality_log> quality_logs;

timespec startProcess, endProcess, endProcessTmp;

timespec diff(timespec start, timespec end);

double to_milliseconds(timespec t);

unsigned long long to_nanoseconds(timespec t);

bool exceeded_time_limit();

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

void initialize()
{
	ifstream is(opts.input_file);

	if(is.is_open())
    {
    	ship ship;
    	berth berth;

    	is >> number_of_ships;

    	for(int i = 1; i <= number_of_ships; i++)
    	{
    		is >> ship.no >> ship.ready_time >> ship.length >> ship.processing_time >> ship.weight >> ship.owner;
    		//preference to big ships
    		//ship.weight = ship.processing_time * ship.length; //~throughput
    		sum_of_weights += ship.weight;
    		ships.push_back(ship);
    	}

    	is >> number_of_berths;

    	for(int i = 1; i <= number_of_berths; i++)
    	{
    		berth.no = i;
    		is >> berth.length;
    		berths.push_back(berth);
    	}
    }

    is.close();
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

bool chain_is_unhandled(array<int, 2> berth, vector<array<int, 4>> &unhandled_chains)
{
	for(vector<array<int, 4>>::iterator itc = unhandled_chains.begin(); itc != unhandled_chains.end(); itc++)
		if((*itc)[0] == berths.at(berth[1]).no && (*itc)[1] == berth[0]) return true;

	return false;
}

array<int, 2> next_berth_index(ship &ship) // at most two ships in one berth!!!
{
	//int berth_index = -1;
	array<int, 2> berth_index = {-1, -1}; // {left (0) / right (1), index}

	for(vector<berth>::iterator itb = berths.begin(); itb != berths.end(); itb++)
	{
		int occupied_length = 0;
		int occupied_side = -1;
		int number_of_handled_ships = 0;
		for(vector<occupied_berth>::iterator itob = occupied_berths.begin(); itob != occupied_berths.end(); itob++)
		{
			if(itb->no == itob->no)
			{
				occupied_length += itob->occupied_length;
				occupied_side = itob->occupied_side;
				++number_of_handled_ships;
			}
		}

		if(number_of_handled_ships < 2 && ship.length <= itb->length - occupied_length)
		{
			if(berth_index[1] == -1)
			{
				//berth_index[0] = 0;
				switch(occupied_side)
				{
					case 0:
						berth_index[0] = 1;
						break;

					case 1:
						berth_index[0] = 0;
						break;

					default:
						berth_index[0] = 0;
				}

				berth_index[1] = itb - berths.begin();
			}
			else
			{
				if(itb->length - occupied_length - ship.length < berths.at(berth_index[1]).length - occupied_length - ship.length)
				{
					switch(occupied_side)
					{
						case 0:
							berth_index[0] = 1;
							break;

						case 1:
							berth_index[0] = 0;
							break;

						default:
							berth_index[0] = 0;
					}

					berth_index[1] = itb - berths.begin();
				}
			}
		}
	}

	return berth_index;
}

array<int, 2> next_berth_index(ship &ship, vector<array<int, 4>> &unhandled_chains) // at most two ships in one berth!!!
{
	//int berth_index = -1;
	array<int, 2> berth_index = {-1, -1}; // {left (0) / right (1), index}

	for(vector<berth>::iterator itb = berths.begin(); itb != berths.end(); itb++)
	{
		int occupied_length = 0;
		int occupied_side = -1;
		int number_of_handled_ships = 0;
		for(vector<occupied_berth>::iterator itob = occupied_berths.begin(); itob != occupied_berths.end(); itob++)
		{
			if(itb->no == itob->no)
			{
				occupied_length += itob->occupied_length;
				occupied_side = itob->occupied_side;
				++number_of_handled_ships;
			}
		}

		if(number_of_handled_ships < 2 && ship.length <= itb->length - occupied_length)
		{
			if(berth_index[1] == -1)
			{
				//berth_index[0] = 0;
				switch(occupied_side)
				{
					case 0:
						berth_index[0] = 1;
						break;

					case 1:
						berth_index[0] = 0;
						break;

					default:
						berth_index[0] = 0;
				}

				berth_index[1] = itb - berths.begin();
			}
			else
			{
				if(itb->length - occupied_length < berths.at(berth_index[1]).length)
				{
					switch(occupied_side)
					{
						case 0:
							berth_index[0] = 1;
							break;

						case 1:
							berth_index[0] = 0;
							break;

						default:
							berth_index[0] = 0;
					}

					berth_index[1] = itb - berths.begin();
				}
			}

			//for(vector<array<int, 2>>::iterator itc = unhandled_chains.begin(); itc != unhandled_chains.end(); itc++)
			//	cout << "no: " << (*itc)[0] << " chain: " << (*itc)[1] << endl;

			if(!chain_is_unhandled(berth_index, unhandled_chains))
			{
				if(berth_index[0] == 0)
				{	
					berth_index[0] = 1; // change chain side (from left to right - always possible)
				}
				else if(berth_index[0] == 1)
				{
					//berth_index = {-1, -1}; // no possible assignment right now, delay needed
					berth_index[0] = -1;
					berth_index[1] = -1;
				}
			}
		}
	}

	return berth_index;
}

void set_available_berths(vector<array<int, 4>> &unhandled_chains)
{
	vector<berth> available_berths;
	berth b;

	for(vector<array<int, 4>>::iterator itc = unhandled_chains.begin(); itc != unhandled_chains.end(); itc++)
	{
		for(vector<berth>::iterator itb = berths.begin(); itb != berths.end(); itb++)
		{
			if((*itc)[0] == itb->no)
			{
				b.no = itb->no;
				b.length = itb->length;
			}
		}

		available_berths.push_back(b);
	}

	berths = available_berths;
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

void detect_collision(vector<processed_berth> &solution)
{
	//bool collision = false;
	int offset = 0;
	int i;
	//vector<processed_ship>::iterator itpbr;

	//arrange_schedule(solution);
	//do
	//{
		for(vector<processed_berth>::iterator itpb = solution.begin(); itpb != solution.end(); ++itpb)
		{
			if(itpb->left.size() > 0)
			{
				for_each(itpb->left.begin(), itpb->left.end(), [&itpb, &offset, &i, &solution](processed_ship & ps1){

					i = 0;
					//offset = 0;

					if(itpb->right.size() > 0)
					{
						for_each(itpb->right.begin(), itpb->right.end(), [&ps1, &itpb, &offset, &i](processed_ship & ps2){

							if(offset == 0)
							{
								if(ps1.start_time >= ps2.start_time && ps1.start_time <= ps2.start_time + ps2.processing_time)
								{
									if(ps1.start_time + ps1.processing_time <= ps2.start_time + ps2.processing_time)
										if(ps1.length + ps2.length > itpb->length) offset = ps1.start_time - ps2.start_time + ps1.processing_time;
									else
										if(ps1.length + ps2.length > itpb->length) offset = ps1.start_time + ps1.processing_time - ps2.start_time + ps2.processing_time + ps2.processing_time;
								}
								else if(ps1.start_time < ps2.start_time)
								{
									if(ps1.start_time + ps1.processing_time >= ps2.start_time && ps1.start_time + ps1.processing_time <= ps2.start_time + ps2.processing_time)
										if(ps1.length + ps2.length > itpb->length) offset = ps1.start_time + ps1.processing_time - ps2.start_time;
									else
										if(ps1.length + ps2.length > itpb->length) offset = ps1.start_time + ps1.processing_time - ps2.start_time + ps2.processing_time + ps2.processing_time;
								}
							}

							if(offset > 0)
								return;
							else
								i++;
						});

						if(offset > 0)
						{
							//cout << itpb->no << ": offset: " << offset << " i: " << i << endl;

							for_each(itpb->right.begin() + i, itpb->right.end(), [&offset](processed_ship & psr){
								//cout << psr.no << ":" << endl;
								//cout << "before: " << psr.start_time << endl;
								psr.start_time += offset;
								psr.flow_time += offset;
								//cout << "after: " << psr.start_time << endl;
							});
							offset = 0;
							detect_collision(solution);
							//collision = true;
						}
					}

					//if(collision == true) return;
				});
			}
		}
	//} while(collision == true);

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
	array<int, 2> berth_index;
	//int berth_index = -1;

	ship_index = next_ship_index();
	ship ship = ships.at(ship_index);

	while(ship.ready_time > global_time)
		global_time++;

	berth_index = next_berth_index(ship);

	if(berth_index[1] != -1)
	{
		processed_ship ps;
		occupied_berth occupied_berth;
		processed_berth pb;
		berth b = berths.at(berth_index[1]);

		pb.length = b.length;

		ps.no = ship.no;
		ps.start_time = global_time;
		//processed_ship.berth_no = berth.no;

		occupied_berth.no = b.no;
		occupied_berth.occupied_length = ship.length;
		occupied_berth.release_time = global_time + ship.processing_time;
		occupied_berth.ship_no = ship.no;

		//sum_of_weights += ship.weight;
		twft += (global_time + ship.processing_time - ship.ready_time) * ship.weight;
		tws += ((global_time + ship.processing_time - ship.ready_time) / ship.processing_time) * ship.weight;
		cmax = global_time + ship.processing_time;

		ps.processing_time = ship.processing_time;
		ps.flow_time = global_time + ship.processing_time - ship.ready_time;
		ps.weight = ship.weight;
		ps.ready_time = ship.ready_time;
		ps.length = ship.length;

		int processed_berth_index = -1;
		for(vector<processed_berth>::iterator itpb = processed_berths.begin(); itpb != processed_berths.end(); itpb++)
		{
			if(itpb->no == b.no)
			{
				processed_berth_index = itpb - processed_berths.begin();
				break;
			}
		}

		if(processed_berth_index != -1)
		{
			pb = processed_berths.at(processed_berth_index);

			if(berth_index[0] == 0)
			{
				pb.left.push_back(ps);
				occupied_berth.occupied_side = 0;
			}
			else
			{
				pb.right.push_back(ps);
				occupied_berth.occupied_side = 1;
			}

			processed_berths.erase(processed_berths.begin() + processed_berth_index);
			processed_berths.push_back(pb);
		}
		else
		{
			pb.no = b.no;

			if(berth_index[0] == 0)
			{
				pb.left.push_back(ps);
				occupied_berth.occupied_side = 0;
			}
			else
			{
				pb.right.push_back(ps);
				occupied_berth.occupied_side = 1;
			}

			processed_berths.push_back(pb);
		}

		//processed_ships.push_back(processed_ship);

		occupied_berths.push_back(occupied_berth);
		ships.erase(ships.begin() + ship_index);
	}
	else
		global_time++;
}

void ship_processing(vector<array<int, 4>> &unhandled_chains)
{
	int ship_index = -1;
	array<int, 2> berth_index;
	//int berth_index = -1;

	ship_index = next_ship_index();
	ship ship = ships.at(ship_index);

	while(ship.ready_time > global_time)
		global_time++;

	berth_index = next_berth_index(ship, unhandled_chains);

	//cout << ship.no << " " << ship.length << endl;//cout << global_time << " " << berth_index[1] << " " << occupied_berths.size() << " " << berths.size() << endl;
	//for(vector<berth>::iterator itb = berths.begin(); itb != berths.end(); itb++)
	//	cout << itb->no << " " << itb->length << endl;

	if(berth_index[1] != -1)
	{
		processed_ship ps;
		occupied_berth occupied_berth;
		processed_berth pb;
		berth b = berths.at(berth_index[1]);

		int unhandled_chain_index = -1;
		for(vector<array<int, 4>>::iterator itc = unhandled_chains.begin(); itc != unhandled_chains.end(); itc++)
			if(b.no == (*itc)[0] && berth_index[0] == (*itc)[1]) unhandled_chain_index = itc - unhandled_chains.begin();
		array<int, 4> uc = unhandled_chains.at(unhandled_chain_index);

		//cout << "before: " << global_time << endl;
		while(uc[2] > global_time) // uc[2] - hole begin
			global_time++;
		//cout << "after: " << global_time << "(" << uc[2] << ", " << uc[3] << ")" << endl;

		pb.length = b.length;

		ps.no = ship.no;
		ps.start_time = global_time;
		//processed_ship.berth_no = berth.no;

		occupied_berth.no = b.no;
		occupied_berth.occupied_length = ship.length;
		occupied_berth.release_time = global_time + ship.processing_time;
		occupied_berth.ship_no = ship.no;

		//sum_of_weights += ship.weight;
		twft += (global_time + ship.processing_time - ship.ready_time) * ship.weight;
		tws += ((global_time + ship.processing_time - ship.ready_time) / ship.processing_time) * ship.weight;
		cmax = global_time + ship.processing_time;

		ps.processing_time = ship.processing_time;
		ps.flow_time = global_time + ship.processing_time - ship.ready_time;
		ps.weight = ship.weight;
		ps.ready_time = ship.ready_time;
		ps.length = ship.length;

		int processed_berth_index = -1;
		for(vector<processed_berth>::iterator itpb = processed_berths.begin(); itpb != processed_berths.end(); itpb++)
		{
			if(itpb->no == b.no)
			{
				processed_berth_index = itpb - processed_berths.begin();
				break;
			}
		}

		if(processed_berth_index != -1)
		{
			pb = processed_berths.at(processed_berth_index);

			if(berth_index[0] == 0)
			{
				pb.left.push_back(ps);
				occupied_berth.occupied_side = 0;
			}
			else
			{
				pb.right.push_back(ps);
				occupied_berth.occupied_side = 1;
			}

			processed_berths.erase(processed_berths.begin() + processed_berth_index);
			processed_berths.push_back(pb);
		}
		else
		{
			pb.no = b.no;

			if(berth_index[0] == 0)
			{
				pb.left.push_back(ps);
				occupied_berth.occupied_side = 0;
			}
			else
			{
				pb.right.push_back(ps);
				occupied_berth.occupied_side = 1;
			}

			processed_berths.push_back(pb);
		}

		//processed_ships.push_back(processed_ship);

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
		array<int, 2> berth_index;

		ship_index = next_ship_index(available_ships);
		//if(ship_index == -1) return false;
		ship ship = available_ships.at(ship_index);

		while(ship.ready_time > global_time)
			global_time++;

		berth_index = next_berth_index(ship);

		if(berth_index[1] != -1)
		{
			processed_ship ps;
			processed_berth pb;
			occupied_berth occupied_berth;
			berth b = berths.at(berth_index[1]);

			pb.length = b.length;

			ps.no = ship.no;
			ps.start_time = global_time;
			//processed_ship.berth_no = berth.no;

			occupied_berth.no = b.no;
			occupied_berth.occupied_length = ship.length;
			occupied_berth.release_time = global_time + ship.processing_time;
			occupied_berth.ship_no = ship.no;

			//sum_of_weights += ship.weight;
			twft += (global_time + ship.processing_time - ship.ready_time) * ship.weight;
			tws += ((global_time + ship.processing_time - ship.ready_time) / ship.processing_time) * ship.weight;
			cmax = global_time + ship.processing_time;

			ps.processing_time = ship.processing_time;
			ps.flow_time = global_time + ship.processing_time - ship.ready_time;
			ps.weight = ship.weight;
			ps.ready_time = ship.ready_time;
			ps.length = ship.length;

			int processed_berth_index = -1;
			for(vector<processed_berth>::iterator itpb = processed_berths.begin(); itpb != processed_berths.end(); itpb++)
			{
				if(itpb->no == b.no)
				{
					processed_berth_index = itpb - processed_berths.begin();
					break;
				}
			}

			if(processed_berth_index != -1)
			{
				pb = processed_berths.at(processed_berth_index);

				if(berth_index[0] == 0)
				{
					pb.left.push_back(ps);
					occupied_berth.occupied_side = 0;
				}
				else
				{
					pb.right.push_back(ps);
					occupied_berth.occupied_side = 1;
				}

				processed_berths.erase(processed_berths.begin() + processed_berth_index);
				processed_berths.push_back(pb);
			}
			else
			{
				pb.no = b.no;

				if(berth_index[0] == 0)
				{
					pb.left.push_back(ps);
					occupied_berth.occupied_side = 0;
				}
				else
				{
					pb.right.push_back(ps);
					occupied_berth.occupied_side = 1;
				}

				processed_berths.push_back(pb);
			}

			//processed_ships.push_back(processed_ship);
			occupied_berths.push_back(occupied_berth);
			available_ships.erase(available_ships.begin() + ship_index);
		}
		else
			break; //no other possible assignment
	}
}

void ship_processing(vector<ship> &available_ships, vector<array<int, 4>> &unhandled_chains)
{
	while(!available_ships.empty())
	{
		int ship_index = -1;
		array<int, 2> berth_index;

		ship_index = next_ship_index(available_ships);
		//if(ship_index == -1) return false;
		ship ship = available_ships.at(ship_index);

		while(ship.ready_time > global_time)
			global_time++;

		berth_index = next_berth_index(ship, unhandled_chains);

		if(berth_index[1] != -1)
		{
			processed_ship ps;
			processed_berth pb;
			occupied_berth occupied_berth;
			berth b = berths.at(berth_index[1]);

			int unhandled_chain_index = -1;
			for(vector<array<int, 4>>::iterator itc = unhandled_chains.begin(); itc != unhandled_chains.end(); itc++)
				if(b.no == (*itc)[0] && berth_index[0] == (*itc)[1]) unhandled_chain_index = itc - unhandled_chains.begin();
			array<int, 4> uc = unhandled_chains.at(unhandled_chain_index);

			//cout << "before: " << global_time << endl;
			while(uc[2] > global_time) // uc[2] - hole begin
				global_time++;
			//cout << "after: " << global_time << "(" << uc[2] << ", " << uc[3] << ")" << endl;

			pb.length = b.length;

			ps.no = ship.no;
			ps.start_time = global_time;
			//processed_ship.berth_no = berth.no;

			occupied_berth.no = b.no;
			occupied_berth.occupied_length = ship.length;
			occupied_berth.release_time = global_time + ship.processing_time;
			occupied_berth.ship_no = ship.no;

			//sum_of_weights += ship.weight;
			twft += (global_time + ship.processing_time - ship.ready_time) * ship.weight;
			tws += ((global_time + ship.processing_time - ship.ready_time) / ship.processing_time) * ship.weight;
			cmax = global_time + ship.processing_time;

			ps.processing_time = ship.processing_time;
			ps.flow_time = global_time + ship.processing_time - ship.ready_time;
			ps.weight = ship.weight;
			ps.ready_time = ship.ready_time;
			ps.length = ship.length;

			int processed_berth_index = -1;
			for(vector<processed_berth>::iterator itpb = processed_berths.begin(); itpb != processed_berths.end(); itpb++)
			{
				if(itpb->no == b.no)
				{
					processed_berth_index = itpb - processed_berths.begin();
					break;
				}
			}

			if(processed_berth_index != -1)
			{
				pb = processed_berths.at(processed_berth_index);

				if(berth_index[0] == 0)
				{
					pb.left.push_back(ps);
					occupied_berth.occupied_side = 0;
				}
				else
				{
					pb.right.push_back(ps);
					occupied_berth.occupied_side = 1;
				}

				processed_berths.erase(processed_berths.begin() + processed_berth_index);
				processed_berths.push_back(pb);
			}
			else
			{
				pb.no = b.no;

				if(berth_index[0] == 0)
				{
					pb.left.push_back(ps);
					occupied_berth.occupied_side = 0;
				}
				else
				{
					pb.right.push_back(ps);
					occupied_berth.occupied_side = 1;
				}

				processed_berths.push_back(pb);
			}

			//processed_ships.push_back(processed_ship);
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

void list_or_look_ahead_scheduling(vector<processed_berth> &solution, vector<array<int, 4>> &unhandled_chains)
{
	processed_berths = solution;
	set_available_berths(unhandled_chains);

	vector<ship> available_ships;

	while(!ships.empty() || !available_ships.empty())
	{
		//if(exceeded_time_limit()) break;

		if(release_berths())
		{
			set_available_ships(available_ships);
			ship_processing(available_ships, unhandled_chains);
			detect_collision(processed_berths);
		}

		set_available_ships(available_ships);
		ship_processing(available_ships, unhandled_chains);
		detect_collision(processed_berths);

		if(!check_ship_availability()) global_time++; //ships with ready_time less than global_time
	}
}

void priority_based_scheduling(vector<processed_berth> &solution, vector<array<int, 4>> &unhandled_chains)
{
	processed_berths = solution; //partial solution
	set_available_berths(unhandled_chains);

	while(!ships.empty())
	{
		//if(exceeded_time_limit()) break;
		// 'zarządzanie' wolnymi/zajętymi nabrzeżami/łańcuchami
		// szeregowanie statków tylko na 'unhandled_chains'
		release_berths();
		ship_processing(unhandled_chains);
		detect_collision(processed_berths);
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

//// additional structure

struct neighbour
{
	vector<processed_berth> solution;
};

//// main

bool compare_processed_ships(const processed_ship &ps1, const processed_ship &ps2)
{
	return ps1.start_time < ps2.start_time;
}

void order_processed_ships()
{
	for(vector<processed_berth>::iterator itpb = processed_berths.begin(); itpb != processed_berths.end(); ++itpb)
	{
		if(itpb->left.size() > 0) sort(itpb->left.begin(), itpb->left.end(), compare_processed_ships);
		if(itpb->right.size() > 0) sort(itpb->right.begin(), itpb->right.end(), compare_processed_ships);
	}
}

void arrange_schedule(vector<processed_berth> &solution)
{
	int _time;

	cmax = 0;

	for(vector<processed_berth>::iterator itpb = solution.begin(); itpb != solution.end(); ++itpb)
	{
		vector<processed_ship> left = itpb->left;
		vector<processed_ship> right = itpb->right;

		_time = 0;

		while(!left.empty())
		{
			processed_ship ps = left.at(0);

			while(ps.ready_time > _time)
				_time++;

			for_each(itpb->left.begin(), itpb->left.end(), [&ps, &_time](processed_ship & p)
			{
				if(p.no == ps.no)
				{
					if(_time + p.processing_time > cmax) cmax = _time + p.processing_time;
					p.start_time = _time;
					p.flow_time = p.start_time + p.processing_time - p.ready_time;
					_time += p.processing_time;
				}
			});

			left.erase(left.begin());
		}

		_time = 0;

		while(!right.empty())
		{
			processed_ship ps = right.at(0);

			while(ps.ready_time > _time)
				_time++;

			for_each(itpb->right.begin(), itpb->right.end(), [&ps, &_time](processed_ship & p)
			{
				if(p.no == ps.no)
				{
					if(_time + p.processing_time > cmax) cmax = _time + p.processing_time;
					p.start_time = _time;
					p.flow_time = p.start_time + p.processing_time - p.ready_time;
					_time += p.processing_time;
				}
			});

			right.erase(right.begin());
		}
	}
}

float compute_twft(vector<processed_berth> &solution)
{
	float l_twft = 0;

	for(vector<processed_berth>::iterator itpb = solution.begin(); itpb != solution.end(); ++itpb)
	{
		if(itpb->left.size() > 0)
		{
			for_each(itpb->left.begin(), itpb->left.end(), [&l_twft](processed_ship & ps)
			{
				l_twft += ps.flow_time * ps.weight;
			});
		}

		if(itpb->right.size() > 0)
		{
			for_each(itpb->right.begin(), itpb->right.end(), [&l_twft](processed_ship & ps)
			{
				l_twft += ps.flow_time * ps.weight;
			});
		}
	}

	return l_twft;
}

float compute_tws(vector<processed_berth> &solution)
{
	float l_tws = 0;

	for(vector<processed_berth>::iterator itpb = solution.begin(); itpb != solution.end(); ++itpb)
	{
		if(itpb->left.size() > 0)
		{
			for_each(itpb->left.begin(), itpb->left.end(), [&l_tws](processed_ship & ps)
			{
				l_tws += (ps.flow_time / ps.processing_time) * ps.weight;
			});
		}

		if(itpb->right.size() > 0)
		{
			for_each(itpb->right.begin(), itpb->right.end(), [&l_tws](processed_ship & ps)
			{
				l_tws += (ps.flow_time / ps.processing_time) * ps.weight;
			});
		}
	}

	return l_tws;
}

int compute_cmax(vector<processed_berth> &solution)
{
	int l_cmax = 0;
	int _time = 0;

	for(vector<processed_berth>::iterator itpb = solution.begin(); itpb != solution.end(); ++itpb)
	{
		if(itpb->left.size() > 0)
		{
			for_each(itpb->left.begin(), itpb->left.end(), [&l_cmax, &_time](processed_ship & ps)
			{
				while(ps.start_time > _time)
					_time++;

				if(_time + ps.processing_time > l_cmax) l_cmax = _time + ps.processing_time;
			});
		}

		_time = 0;

		if(itpb->right.size() > 0)
		{
			for_each(itpb->right.begin(), itpb->right.end(), [&l_cmax, &_time](processed_ship & ps)
			{
				while(ps.start_time > _time)
					_time++;

				if(_time + ps.processing_time > l_cmax) l_cmax = _time + ps.processing_time;
			});
		}
	}

	return l_cmax;
}

float evaluate_solution(vector<processed_berth> &solution)
{
	float l_twft;
	l_twft = compute_twft(solution);
	//return 1.0 / ((1.0 / sum_of_weights) * l_twft);
	return (1.0 / sum_of_weights) * l_twft;
}

int get_number_of_chains(vector<processed_berth> &solution)
{
	int number_of_chains = 0;

	for_each(solution.begin(), solution.end(), [&number_of_chains](processed_berth & pb){
		if(pb.left.size() > 0) ++number_of_chains;
		if(pb.right.size() > 0) ++number_of_chains;
	});

	return number_of_chains;
}

vector<array<int, 4>> destroy_part_of_solution(vector<processed_berth> &solution, vector<ship> &unhandled_ships)
{
	int fraction = 0;
	array<int, 4> unhandled_chain = {0, 0, 0, 0}; // berth #; chain: 0 - left, 1 - right; hole begin; hole end
	vector<array<int, 4>> unhandled_chains;

	if(opts.destruction_method == "A")
	{
		if(number_of_berths > 1 && solution.size() > 1) // rules: at least 2 chains from at least 2 berths --- else? skip & return basic solution?
		{
			int number_of_chains = get_number_of_chains(solution);
			//vector<array<int, 2>> chains_for_destruction;

			fraction = (int) round(opts.destruction_fraction * number_of_chains);
			if(fraction < 2) fraction = 2;

			//array<int, 2> previous_chain = {0, 0}; // {berth, chain: 0 - left, 1 - right}
			//array<int, 2> current_chain = {0, 0};
			int previous_berth_no = 0;
			int destroyed_chains = 0;
			int decision = 0;
			while(destroyed_chains < fraction)
			{

				for(vector<processed_berth>::iterator its = solution.begin(); its != solution.end(); ++its)
				{
					if(fraction == 2 && previous_berth_no == its->no)
					{
						continue;
					}
					else
					{
						if(its->left.size() > 0)
						{
							decision = rand() % 2;

							if(decision == 1)
							{
								// ... dismount chains, etc.
								ship s;
								for_each(its->left.begin(), its->left.end(), [&s, &unhandled_ships](processed_ship & ps){
									s.no = ps.no;
									s.ready_time = ps.ready_time;
									s.processing_time = ps.processing_time;
									s.length = ps.length;
									s.weight = ps.weight;
									s.owner = 1;//!
									unhandled_ships.push_back(s);
								});

								its->left.clear();

								//cout << "berth: " << its->no << " chain: left unhandled_ships: " << unhandled_ships.size() << endl;

								previous_berth_no = its->no;
								++destroyed_chains;

								unhandled_chain[0] = its->no;
								unhandled_chain[1] = 0;
								unhandled_chains.push_back(unhandled_chain);

								if(destroyed_chains >= fraction) break;
							}
						}

						if(its->right.size() > 0)
						{
							decision = rand() % 2;

							if(decision == 1)
							{
								// ... dismount chains, etc.
								ship s;
								for_each(its->right.begin(), its->right.end(), [&s, &unhandled_ships](processed_ship & ps){
									s.no = ps.no;
									s.ready_time = ps.ready_time;
									s.processing_time = ps.processing_time;
									s.length = ps.length;
									s.weight = ps.weight;
									s.owner = 1;//!
									unhandled_ships.push_back(s);
								});

								its->right.clear();

								//cout << "berth: " << its->no << " chain: right unhandled_ships: " << unhandled_ships.size() << endl;

								previous_berth_no = its->no;
								++destroyed_chains;

								unhandled_chain[0] = its->no;
								unhandled_chain[1] = 1;
								unhandled_chains.push_back(unhandled_chain);

								if(destroyed_chains >= fraction) break;
							}
						}

					}

				}

				//cout << "unhandled_chains: " << unhandled_chains.size() << endl;

			}
		}
	}
	else
	{
		map<int, array<int, 2>> rnd;
		map<int, array<int, 2>> rnd_ships_per_berth;
		int rnd_left = 0;
		int rnd_right = 0;
		int sum_of_rnds = 0;
		int sum_of_ships_on_berths = 0;
		int ship_position = 0;
		int sum_of_fractions = 0;
		int decision = 0;
		int rnd_range = 0;

		fraction = (int) round(opts.destruction_fraction * number_of_ships);

		do
		{
			sum_of_ships_on_berths = 0;

			for(vector<processed_berth>::iterator its = solution.begin(); its != solution.end(); ++its)
			{
				//rnd[its->no] = {-1, -1};
				//rnd_ships_per_berth[its->no] = {0, 0};
				rnd[its->no][0] = -1;
				rnd[its->no][1] = -1;
				rnd_ships_per_berth[its->no][0] = 0;
				rnd_ships_per_berth[its->no][1] = 0;

				if(its->left.size() > 0)
				{
					rnd_left = rand() % 2;
					rnd[its->no][0] = rnd_left;
					sum_of_rnds += rnd_left;//++sum_of_rnds;
					if(rnd_left > 0)
						sum_of_ships_on_berths += its->left.size();
				}

				if(its->right.size() > 0)
				{
					rnd_right = rand() % 2;
					rnd[its->no][1] = rnd_right;
					sum_of_rnds += rnd_right;//++sum_of_rnds;
					if(rnd_right > 0)
						sum_of_ships_on_berths += its->right.size();
				}
			}
		}while(sum_of_ships_on_berths < fraction);

		//cout << "fraction: " << fraction << " sum of ships: " << sum_of_ships_on_berths << endl;

		while(fraction > 0)
		{
			for(vector<processed_berth>::iterator its = solution.begin(); its != solution.end(); ++its)
			{
				if(rnd[its->no][0] != -1)
				{
					decision = rand() % 2;

					if(decision == 1)
					{
						if(rnd_ships_per_berth[its->no][0] < its->left.size())
						{
							rnd_ships_per_berth[its->no][0]++;
							fraction--;
						}
					}
				}

				if(fraction <= 0) break;

				if(rnd[its->no][1] != -1)
				{
					decision = rand() % 2;

					if(decision == 1)
					{
						if(rnd_ships_per_berth[its->no][1] < its->right.size())
						{
							rnd_ships_per_berth[its->no][1]++;
							fraction--;
						}
					}
				}

				if(fraction <= 0) break;
			}
		}

		//tmp - just to `cout` it
		/*cout << "---\n";
		for(vector<processed_berth>::iterator its = solution.begin(); its != solution.end(); ++its)
		{
			cout << its->no << ": left[" << rnd_ships_per_berth[its->no][0] << "] : right[" << rnd_ships_per_berth[its->no][1] << "]\n";
		}
		cout << "---\n";*/

		for(vector<processed_berth>::iterator its = solution.begin(); its != solution.end(); ++its)
		{
			ship_position = 0;
			rnd_range = 0;

			if(rnd_ships_per_berth[its->no][0] > 0)
			{
				rnd_range = its->left.size() - rnd_ships_per_berth[its->no][0];
				if(rnd_range == 0)
					ship_position = 0;
				else
					ship_position = rand() % rnd_range;

				ship s;
				for_each(its->left.begin() + ship_position, its->left.begin() + ship_position + rnd_ships_per_berth[its->no][0], [&s, &unhandled_ships](processed_ship & ps){
					s.no = ps.no;
					s.ready_time = ps.ready_time;
					s.processing_time = ps.processing_time;
					s.length = ps.length;
					s.weight = ps.weight;
					s.owner = 1;//!
					unhandled_ships.push_back(s);
				});

				unhandled_chain[0] = its->no;
				unhandled_chain[1] = 0;
				unhandled_chain[2] = (its->left.begin() + ship_position)->start_time;
				unhandled_chain[3] = (its->left.begin() + ship_position + rnd_ships_per_berth[its->no][0])->start_time + (its->left.begin() + ship_position + rnd_ships_per_berth[its->no][0])->processing_time;
				//cout << "left hole begin: " << (its->left.begin() + ship_position)->start_time << " left hole end: " << (its->left.begin() + ship_position + rnd_ships_per_berth[its->no][0])->start_time + (its->left.begin() + ship_position + rnd_ships_per_berth[its->no][0])->processing_time << endl;
				unhandled_chains.push_back(unhandled_chain);

				its->left.erase(its->left.begin() + ship_position, its->left.begin() + ship_position + rnd_ships_per_berth[its->no][0]);
			}

			ship_position = 0;
			rnd_range = 0;

			if(rnd_ships_per_berth[its->no][1] > 0)
			{
				rnd_range = its->right.size() - rnd_ships_per_berth[its->no][1];
				if(rnd_range == 0)
					ship_position = 0;
				else
					ship_position = rand() % rnd_range;

				ship s;
				for_each(its->right.begin() + ship_position, its->right.begin() + ship_position + rnd_ships_per_berth[its->no][1], [&s, &unhandled_ships](processed_ship & ps){
					s.no = ps.no;
					s.ready_time = ps.ready_time;
					s.processing_time = ps.processing_time;
					s.length = ps.length;
					s.weight = ps.weight;
					s.owner = 1;//!
					unhandled_ships.push_back(s);
				});

				unhandled_chain[0] = its->no;
				unhandled_chain[1] = 1;
				unhandled_chain[2] = (its->right.begin() + ship_position)->start_time;
				unhandled_chain[3] = (its->right.begin() + ship_position + rnd_ships_per_berth[its->no][1])->start_time + (its->right.begin() + ship_position + rnd_ships_per_berth[its->no][1])->processing_time;
				//cout << "right hole begin: " << (its->right.begin() + ship_position)->start_time << " right hole end: " << (its->right.begin() + ship_position + rnd_ships_per_berth[its->no][1])->start_time + (its->right.begin() + ship_position + rnd_ships_per_berth[its->no][1])->processing_time << endl;
				unhandled_chains.push_back(unhandled_chain);

				its->right.erase(its->right.begin() + ship_position, its->right.begin() + ship_position + rnd_ships_per_berth[its->no][1]);
			}
		}
	}

	return unhandled_chains;
}

void ils()
{
	vector<processed_berth> solution;
	vector<processed_berth> initial_solution;
	vector<processed_berth> best_solution;
	quality_log ql;

	float initial_solution_evaluation;
	float previous_solution_evaluation;
	//float final_solution_evaluation = 0;

	vector<ship> unhandled_ships;
	vector<berth> tmp_berths = berths;
	vector<array<int, 4>> unhandled_chains;
	//vector<processed_berth> new_solution;
	int future_arrivals[4] = {1, 2, 5, 10};

	solution = initial_solutions.front();
	initial_solution_evaluation = evaluate_solution(solution);
	initial_solution = solution;
	previous_solution_evaluation = 0;

	for_each(initial_solutions.begin()+1, initial_solutions.end(), [&initial_solution_evaluation, &solution, &initial_solution, &ql](vector<processed_berth> & s){
		if(initial_solution_evaluation > evaluate_solution(s))
		{
			initial_solution_evaluation = evaluate_solution(s);
			solution = s;
			initial_solution = s;
			clock_gettime(CLOCK_MONOTONIC, &endProcess);
			ql.mwft = initial_solution_evaluation;
			ql.actual_time = endProcess;
			quality_logs.push_back(ql);
		}
	});

	//cout << "best initial: " << evaluate_solution(solution) << endl;

	do
	{
		//while(!initial_solutions.empty())
		//{
			//solution = initial_solutions.front();

			unhandled_chains = destroy_part_of_solution(solution, unhandled_ships);
			//cout << "unhandled ships: " << unhandled_ships.size() << endl;
			
			//reconstruction
			for_each(scheduling_policies.begin(), scheduling_policies.end(), [&solution, &unhandled_ships, &tmp_berths, &unhandled_chains, &future_arrivals](string policy) -> bool{	
				if(exceeded_time_limit()) return true;
				//new_solution = reconstruct_solution(policy, solution, unhandled_ships, tmp_berths, unhandled_chains); //the best from reconstructed solutions
				//final_solutions.push_back(new_solution);
				opts.scheduling_policy = policy;
				global_time = 0;
				//sum_of_weights = 0;
				processed_berths.clear();
				occupied_berths.clear();
				ships = unhandled_ships;
				berths = tmp_berths;

				priority_based_scheduling(solution, unhandled_chains);
				order_processed_ships();
				arrange_schedule(processed_berths); // shift left to close holes
				detect_collision(processed_berths);
				final_solutions.push_back(processed_berths);

				for(int i = 0; i < sizeof(future_arrivals)/sizeof(int); i++)
				{
					if(exceeded_time_limit()) break;
					//new_solution = reconstruct_solution(policy, solution, unhandled_ships, tmp_berths, unhandled_chains, future_arrivals[i]); //the best from reconstructed solutions
					//final_solutions.push_back(new_solution);
					//opts.scheduling_policy = policy;
					opts.future_arrivals = future_arrivals[i];
					global_time = 0;
					//sum_of_weights = 0;
					processed_berths.clear();
					occupied_berths.clear();
					ships = unhandled_ships;
					berths = tmp_berths;

					list_or_look_ahead_scheduling(solution, unhandled_chains);
					order_processed_ships();
					arrange_schedule(processed_berths);
					detect_collision(processed_berths);
					final_solutions.push_back(processed_berths);
				}
			});
			
			//initial_solutions.erase(initial_solutions.begin());
			//unhandled_ships.clear();
		//}

		if(final_solutions.size() > 0)
		{
			////solution = final_solutions.front();
			////previous_solution_evaluation = evaluate_solution(solution);

			if(best_solution.empty())
			{
				previous_solution_evaluation = initial_solution_evaluation;////previous_solution_evaluation = evaluate_solution(solution);
				best_solution = initial_solution;
				clock_gettime(CLOCK_MONOTONIC, &endProcess);
				ql.mwft = previous_solution_evaluation;
				ql.actual_time = endProcess;
				quality_logs.push_back(ql);
			}

			//previous = solution;
			//twft = compute_twft(solution);
			//tws = compute_tws(solution);
			//cmax = compute_cmax(solution);
			
			for_each(final_solutions.begin(), final_solutions.end(), [&previous_solution_evaluation, &best_solution, &ql](vector<processed_berth> & solution) -> bool{
				if(exceeded_time_limit()) return true;
				if(previous_solution_evaluation > evaluate_solution(solution))
				{
					if(evaluate_solution(best_solution) > evaluate_solution(solution))
					{
						previous_solution_evaluation = evaluate_solution(solution);
						best_solution = solution;
						clock_gettime(CLOCK_MONOTONIC, &endProcess);
						ql.mwft = previous_solution_evaluation;
						ql.actual_time = endProcess;
						quality_logs.push_back(ql);
					}
					//twft = compute_twft(solution);
					//tws = compute_tws(solution);
					//cmax = compute_cmax(solution);
				}
			});
		}
		
		//cout << "best current: " << evaluate_solution(best_solution) << endl;

		solution = best_solution;
		//initial_solutions.clear();
		final_solutions.clear();
		unhandled_ships.clear();
		unhandled_chains.clear();
		occupied_berths.clear();
		//initial_solutions.push_back(processed_berths);
	} while(!exceeded_time_limit());

	processed_berths = best_solution;

	/*if(!exceeded_time_limit())
	{
		cout << "not exceeded" << endl;
		cout << "[---solution begin---]" << endl;
		for(vector<processed_berth>::iterator itpbtmp = processed_berths.begin(); itpbtmp != processed_berths.end(); ++itpbtmp)
		{
			cout << itpbtmp->no << ":" << endl;//" " << itpbtmp->left.size() << " " << itpbtmp->right.size() << endl;//cout << itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;
			if(!itpbtmp->left.empty())
			{
				cout << "\tleft:" << endl;
				for(vector<processed_ship>::iterator itps = itpbtmp->left.begin(); itps != itpbtmp->left.end(); itps++)
					cout << "\t\t" << itps->no << " " << itps->start_time << " " << itps->processing_time << " " << itps->flow_time << endl;
			}
			if(!itpbtmp->right.empty())
			{
				cout << "\tright:" << endl;
				for(vector<processed_ship>::iterator itps = itpbtmp->right.begin(); itps != itpbtmp->right.end(); itps++)
					cout << "\t\t" << itps->no << " " << itps->start_time << " " << itps->processing_time << " " << itps->flow_time << endl;
			}
		}
		cout << "[---solution end---]" << endl;
		initial_solutions.clear();
		final_solutions.clear();
		unhandled_ships.clear();
		unhandled_chains.clear();
		occupied_berths.clear();
		initial_solutions.push_back(processed_berths);
		ils();
	}*/
}

bool compare_processed_berths(const processed_berth &b1, const processed_berth &b2)
{
	return b1.no < b2.no;
}

float schedule()
{
	float mwft = 0;
	float mws = 0;
	vector<ship> tmp;

	timespec elapsedTime;

	// initialize();
	tmp = ships;

	clock_gettime(CLOCK_MONOTONIC, &startProcess);

	for_each(scheduling_policies.begin(), scheduling_policies.end(), [&tmp](string policy){
		opts.scheduling_policy = policy;

		priority_based_scheduling();
		initial_solutions.push_back(processed_berths);

		int future_arrivals[4] = {1, 2, 5, 10};

		for(int i = 0; i < sizeof(future_arrivals)/sizeof(int); i++)
		{
			ships = tmp;
			global_time = 0;
			//sum_of_weights = 0;
			processed_berths.clear();
			occupied_berths.clear();
			opts.future_arrivals = future_arrivals[i];
			list_or_look_ahead_scheduling();
			initial_solutions.push_back(processed_berths);
		}
	});

	ils();

	//if(endProcess.tv_sec != 0 && endProcess.tv_nsec != 0)
	elapsedTime = diff(startProcess, endProcess);
	//else
	//	elapsedTime = diff(startProcess, endProcessTmp);

	twft = compute_twft(processed_berths);
	tws = compute_tws(processed_berths);
	cmax = compute_cmax(processed_berths);

	mwft = (1.0 / sum_of_weights) * twft;
	mws = (1.0 / sum_of_weights) * tws;

	// ofstream output_file(opts.output_file);

	// sort(processed_berths.begin(), processed_berths.end(), compare_processed_berths);

	// output_file << to_milliseconds(elapsedTime) << " " << fixed << setprecision(4) << mwft << " " << cmax << " " << mws << endl; //runtime.count()
	// output_file << number_of_berths << " " << number_of_ships << endl;
	// for(vector<processed_berth>::iterator itpb = processed_berths.begin(); itpb != processed_berths.end(); itpb++)
	// {
	// 	output_file << itpb->no << ":" << endl;//" " << itpb->left.size() << " " << itpb->right.size() << endl;//output_file << itps->no << " " << itps->start_time << " " << itps->berth_no << " " << itps->processing_time << " " << itps->flow_time << endl;
	// 	if(!itpb->left.empty())
	// 	{
	// 		output_file << "\tleft:" << endl;
	// 		for(vector<processed_ship>::iterator itps = itpb->left.begin(); itps != itpb->left.end(); itps++)
	// 			output_file << "\t\t" << itps->no << " " << itps->start_time << " " << itps->processing_time << " " << itps->flow_time << endl;
	// 	}
	// 	if(!itpb->right.empty())
	// 	{
	// 		output_file << "\tright:" << endl;
	// 		for(vector<processed_ship>::iterator itps = itpb->right.begin(); itps != itpb->right.end(); itps++)
	// 			output_file << "\t\t" << itps->no << " " << itps->start_time << " " << itps->processing_time << " " << itps->flow_time << endl;
	// 	}
	// }
	// output_file.close();

	// opts.output_file += ".log";

	// ofstream log_file(opts.output_file);
	// for(vector<quality_log>::iterator itql = quality_logs.begin(); itql != quality_logs.end(); itql++)
	// {
	// 	log_file << to_milliseconds(diff(startProcess, itql->actual_time)) << " " << fixed << setprecision(4) << itql->mwft << endl;
	// }
	// log_file.close();
	return mwft;
}

int main(int argc, char *argv[])
{
	srand(time(0)); //random seed

	opts.input_file = "";
	opts.output_file = "";
	opts.scheduling_policy = "";
	opts.future_arrivals = 0;

	opts.destruction_method = "C";
	opts.destruction_fraction = 0.1;

	int opt;

	while((opt = getopt(argc, argv, "i:o:p:k:m:d:h?")) != -1)
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

			case 'm':
				opts.destruction_method = optarg;
				break;

			case 'd':
				opts.destruction_fraction = atof(optarg);
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

    schedule();

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

bool exceeded_time_limit()
{
	clock_gettime(CLOCK_MONOTONIC, &endProcessTmp);
	if(diff(startProcess, endProcessTmp).tv_sec > TIME_LIMIT) return true;
	return false;
}

double schedule(const vector<ship> &ships_input, const vector<berth> &berths_input) {
	global_time = 0;
	twft = 0;
	tws = 0;
	cmax = 0;
	processed_berths.clear();
	occupied_berths.clear();
	initial_solutions.clear();
	final_solutions.clear();
	quality_logs.clear();

	ships = ships_input;
	berths = berths_input;
	number_of_ships = ships_input.size();
	number_of_berths = berths_input.size();
	sum_of_weights = 0;
	for (size_t i = 0; i < ships_input.size(); ++i) {
		sum_of_weights += ships_input[i].weight;
	}

	return schedule();
}

}
