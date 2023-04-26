// MAIN FILE

#include "utils.h"
#include "solver.h"
#include "plot.h"

int main(int argc, char** argv) {
    instance inst;

    parse_command_line(argc,argv, &inst);

    parse_model(&inst);

    compute_distances(&inst);

    solve(&inst);
   
    plot_solution(&inst);

    free_instance(&inst);
    
    return 0;
}
