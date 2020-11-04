#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <string>
#include <algorithm> 
#include "evaluator.cpp"
#include <mpi.h>

using namespace std;



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

    Evaluator simple_eval;


    simple_eval.set_num_instances(8);

    float sum_mwft_proc = simple_eval.calculateMWFT(argc,argv);
    if(sum_mwft_proc != 0)
    cout << "total mwft on processor: " << sum_mwft_proc << endl;
    
    //simple_eval.calculateMWFT(argc,argv);

    //if(MPI::Is_finalized());
    //cout << "total mwft on processor: " << simple_eval.get_mwft_from_one_processor() << endl;
    //schedule();

    return EXIT_SUCCESS;
}

