// MAIN FILE

#include "utils.h"
#include "heuristics.h"
#include "metaheuristics.h"
#include "plot.h"

int main(int argc, char** argv) {
    instance inst;

    parse_command_line(argc,argv, &inst);

    parse_model(&inst);

    compute_distances(&inst);

    //execute algorithms
    greedy(&inst, 0);
    //greedy_iterative(&inst);

    //extra_mileage(&inst);
    
    //alg_2opt(&inst);

    //greedy_2opt(&inst, 20);
    //extra_mileage_2opt(&inst);
    //grasp_2opt(&inst, 0, .5);

    //tabu_search(&inst);
    
    vns(&inst);

    //grasp_iterative(&inst);

    plot_solution(&inst);

    free_instance(&inst);
    
    return 0;
}
