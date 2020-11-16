#ifndef ILS_H
#define ILS_H

#include <string>
#include <vector>
#include "ship.h"

namespace ils {
struct options
{
    std::string input_file; //-i
    std::string output_file; //-o
    std::string scheduling_policy; //-p
    int future_arrivals; //-k
    std::string destruction_method; //-m
    float destruction_fraction; //-d
};

extern options opts;

double schedule(const std::vector<ship> &ships_input, const std::vector<berth> &berths);

}

#endif
