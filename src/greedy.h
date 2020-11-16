#ifndef GREEDY_H
#define GREEDY_H

#include <vector>
#include "ship.h"

namespace greedy {
struct options
{
    std::string input_file; //-i
    std::string output_file; //-o
    std::string scheduling_policy; //-p
	int future_arrivals; //-k
};

extern options opts;

double schedule(const std::vector<ship> &ships_input, const std::vector<berth> &berths);

}

#endif
