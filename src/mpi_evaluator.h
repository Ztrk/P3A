#ifndef MPI_EVALUATOR_H
#define MPI_EVALUATOR_H

#include <fstream>
#include <vector>
#include "evaluator_interface.h"
#include "evaluator.h"

class MpiEvaluator : public EvaluatorInterface {
public:
    MpiEvaluator(int num_instances, Evaluator &evaluator) 
    : num_instances(num_instances), evaluator(evaluator), log("evaluator.log") { }

    double evaluate(const std::vector<int> &berth_frequencies, 
                    const std::vector<int> &berth_lengths);
    
    void listen();
    void stop_listeners();

private:
    int num_instances;
    Evaluator &evaluator;
    const int MPI_TAG = 0;
    const int ROOT = 0;
    const int EXIT = -1;

    double standard_deviation(const std::vector<double> &mwft);
    double median(std::vector<double> x);
    std::vector<double> quartiles(const std::vector<double> &mwft);
    double min(const std::vector<double> &x);
    double max(const std::vector<double> &x);

    std::ofstream log;

    bool instances_generated = false;
};

#endif
