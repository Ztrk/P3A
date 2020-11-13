#ifndef INSTANCE_GENERATOR_H
#define INSTANCE_GENERATOR_H

#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace generator {
const std::string config_file = "lh_instance.json";

struct ship {
    int no; //# of ship
    int ready_time; //ready time
    int length; //length
    int processing_time; //processing time
    int weight; //weight
    int owner = 1; //owner
};

std::vector<ship> generate_instance(nlohmann::json &instance_parameters);

inline std::vector<ship> generate_instance() {
    nlohmann::json instance_parameters;
    std::ifstream instance_parameters_file(config_file);
    instance_parameters_file >> instance_parameters;
    return generate_instance(instance_parameters);
}

}

#endif
