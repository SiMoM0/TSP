// MAIN FILE

#include "utils.h"
#include "heuristics.h"
#include "plot.h"

int main(int argc, char** argv) {

    //TODO move these lines into parse_command_line (?)
    if(argc<2){
        printf("Usage: %s -help for the available commands\n", argv[0]);
        exit(1);
    }

    // tsp instance
    instance inst;
    
    // read user commands
    parse_command_line(argc,argv, &inst);

    // read tsp instance
    parse_model(&inst);

    //execute algorithm
    greedy_iterative(&inst);

    //plot tsp instance
    plot_solution(&inst);

    free_instance(&inst);
    
    return 0;
}
