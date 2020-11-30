#ifndef INSTANCE_GENERATOR_H
#define INSTANCE_GENERATOR_H

#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "ship.h"

namespace generator {
const std::string config_file = "lh_instance.json";

std::vector<ship> generate_instance(nlohmann::json &instance_parameters);

inline std::vector<ship> generate_instance() {
    nlohmann::json instance_parameters;
    std::ifstream instance_parameters_file(config_file);
    instance_parameters_file >> instance_parameters;
    return generate_instance(instance_parameters);
}

}

class BaseInstanceGenerator {
public:
    virtual std::vector<ship> generate(int instance_num) = 0;
};

#endif
