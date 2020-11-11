#ifndef MPI_EVALUATOR_H
#define MPI_EVALUATOR_H

#include <vector>
#include "evaluator_interface.h"
#include "evaluator.h"

class MpiEvaluator : public EvaluatorInterface {
public:
    MpiEvaluator(int num_instances) : num_instances(num_instances) { }

    double evaluate(const std::vector<int> &berth_frequencies, 
                    const std::vector<int> &berth_lengths);
    
    void listen();
    void stop_listeners();
private:

    Evaluator evaluator;
    int num_instances;
    const int MPI_TAG = 0;
    const int ROOT = 0;
    const int EXIT = -1;
};

#endif
