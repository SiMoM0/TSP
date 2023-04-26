#include "tspCplex.h"


void build_model(instance* inst, CPXENVptr env, CPXLPptr lp) {
    double zero = 0.0;
    char binary = 'B';

    char** cname = (char**) calloc(1, sizeof(char*));
    cname[0] = (char*) calloc(100, sizeof(char));

    //add binary variables
    for(int i=0; i<inst->nnodes; ++i) {
        for(int j=i+1; j<inst->nnodes; ++j) {
            sprintf(cname[0], "x(%d,%d)", i+1, j+1);
            double obj = get_cost(i, j, inst);
            double lb = 0.0;
            double ub = 1.0;

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname);
            if(status)
                print_error("Wrong CPXnewcols on x var.s");

            int cols = CPXgetnumcols(env, lp);  
            if(cols-1 != xpos(i, j, inst))
                print_error("Wrong position for x var.s");
        }
    }

    //add degree constraints
    int* index = (int*) calloc(inst->nnodes, sizeof(int));
    double* value = (double*) calloc(inst->nnodes, sizeof(double));

    for(int i=0; i<inst->nnodes; ++i) {
        double rhs = 2.0;
        char sense = 'E';
        sprintf(cname[0], "degree(%d)", i+1);
        int nnz = 0;

        for(int j=0; j<inst->nnodes; ++j) {
            if(j == i) continue;
            index[nnz] = xpos(j, i, inst);
            value[nnz] = 1.0;
            nnz++;
        }

        int izero = 0;
        int status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]);
        if(status)
            print_error("CPXaddrows(): error 1");
    }

    free(value);
    free(index);

    free(cname[0]);
    free(cname);

    //save lp file
    if(inst->verbose >= 100)
        CPXwriteprob(env, lp, "model.lp", NULL);
}
//Build_sol transforms an integer sol of the model (given in cplex format) to an arry of succ and an array of connected components
void build_sol(const double *xstar, instance *inst, int * succ, int* comp, int* ncomp){
    
    //initialiazation of succ, comp and ncomp
    *ncomp = 0;
	for (int i = 0; i < inst->nnodes; i++){
		succ[i] = -1;
		comp[i] = -1;
	}


	for (int start = 0; start < inst->nnodes; start++){
		if (comp[start] >= 0)
			continue; // node "start" already visited -> skip

		(*ncomp)++; //new component found
		int i = start;
		int done = 0;
		while (!done){ // go and visit the current component
			comp[i] = *ncomp;
			done = 1;
			for (int j = 0; j < inst->nnodes; j++){
				if (i != j && xstar[xpos(i, j, inst)] > 0.5 && comp[j] == -1){ // the edge [i,j] is selected in xstar and j was not visited before
					succ[i] = j;
					i = j;
					done = 0;
					break;
				}
			}
		}
		succ[i] = start; // last arc to close the cycle
						 // go to the next component...
	}


    
}

void set_params(instance* inst, CPXENVptr env){
    //TO DO
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);

	if (inst->verbose >= 10) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen


    if (inst->timelimit > 0) { // Time limits <= 0 not allowed
        CPXsetdblparam(env, CPXPARAM_TimeLimit, inst->timelimit);
    }
    if (inst->randomseed >= 0) {
        CPXsetintparam(env, CPX_PARAM_RANDOMSEED, inst->randomseed);
    }
    /*if (params.num_threads > 0) {
        CPXsetintparam(env, CPXPARAM_Threads, params.num_threads);
    }*/

    // Cplex precision
    CPXsetdblparam(env, CPX_PARAM_EPINT, 0.0);		
	CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-9);	 
	CPXsetdblparam(env, CPX_PARAM_EPRHS, 1e-9); 
}
int TSPopt(instance *inst){
    int error;
    //debug
    printf("sto aprendo cplex\n");
    getchar();

    //open cplex model
    CPXENVptr env = CPXopenCPLEX(&error);       // generate new environment, in err will be saved errors
    if(error) print_error("CPXopenCPLEX() error\n");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");   // create new empty linear programming problem (no variables, no constraints ...)
    if(error) print_error("CPXcreateprob() error\n");

    printf("ho aperto cplex\n");
    getchar();
    // Build the model (build the cplex model)
    build_model(inst, env, lp);

    //debug
    printf("ho fatto la build del model\n");
    getchar();

    //cplex's parameters setting
    set_params(inst, env);
    //debug
    printf("ho fatto set_params\n");
    getchar();

    //Start counting time
    struct timeval start, end;
    gettimeofday(&start, 0);


    //Optimize the model (the solution is stored inside the env variable)
    printf("i am solving the problem:\n");
    int status = solve_problem(env, lp, inst);

    //Compute elapsed time
    gettimeofday(&end, 0);
    double elapsed = get_elapsed_time(start, end);

    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

