#ifndef EVALUATOR_INTERFACE_H
#define EVALUATOR_INTERFACE_H

#include <vector>

class EvaluatorInterface {
public:
    virtual double evaluate(const std::vector<int> &berth_frequencies, 
                            const std::vector<int> &berth_lengths) = 0;
};

#endif
