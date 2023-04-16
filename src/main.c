// MAIN FILE

#include "utils.h"
#include "heuristics.h"
#include "metaheuristics.h"
#include "solver.h"
#include "plot.h"

int main(int argc, char** argv) {
    instance inst;

    parse_command_line(argc,argv, &inst);

    parse_model(&inst);

    compute_distances(&inst);

    solve_instance(&inst);

    plot_solution(&inst);

    free_instance(&inst);
    
    return 0;
}
