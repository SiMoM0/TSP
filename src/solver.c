#include "solver.h"

void solve(instance* inst){
    if(inst->cplex == 1) {
        int error = TSPopt(inst);
        if(error) print_error("Execution of TSPopt FAILED");
    } else {
        solve_heuristic(inst);
    }

    if(inst->verbose >= 1)
        printf("Solution value = %f\n", inst->zbest);
}

int solve_problem(CPXENVptr env, CPXLPptr lp, instance *inst) {
    int status;

    if(strncmp(inst->solver, "BENDERS", 7) == 0) {
        status = benders(inst, env, lp);
    } else if(strncmp(inst->solver, "BRANCH_CUT_RLX", 14) == 0) {
        status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_RELAXATION);
    } else if(strncmp(inst->solver, "BRANCH_CUT", 10) == 0) {
        status = branch_and_cut(inst, env, lp, CPX_CALLBACKCONTEXT_CANDIDATE); 
    } else if(strncmp(inst->solver, "HARD_FIX", 8) == 0) {
        status = hard_fixing(inst, env, lp);
    } else if(strncmp(inst->solver, "LOCAL_BRANCH", 12) == 0) {
        status = local_branching(inst, env, lp);
    } else {
        print_error("Invalid solver selected");
    }

    if(status)
        print_error("Execution FAILED");
    else if(status == 2)
        print_error("Time out during execution");

    return status;
}

void solve_heuristic(instance* inst) {
    if(strcmp(inst->solver, "GREEDY") == 0) {
        //TODO how to select starting node
        greedy(inst, 0);
    } else if(strcmp(inst->solver, "GRASP") == 0) {
        grasp(inst, 0, 0.8);
    } else if(strcmp(inst->solver, "EXTRA_MIL") == 0) {
        extra_mileage(inst);
    } else if(strcmp(inst->solver, "GREEDY_ITER") == 0) {
        greedy_iterative(inst);
    } else if(strcmp(inst->solver, "GRASP_ITER") == 0) {
        grasp_iterative(inst);
    } else if(strcmp(inst->solver, "GREEDY_2OPT") == 0) {
        greedy_2opt(inst, 0);
    } else if(strcmp(inst->solver, "EXTRA_MIL_2OPT") == 0) {
        extra_mileage_2opt(inst);
    } else if(strcmp(inst->solver, "GRASP_2OPT") == 0) {
        grasp_2opt(inst, 0, 0.8);
    } else if(strcmp(inst->solver, "TABU_SEARCH") == 0) {
        //TODO choose initial solver
        greedy_iterative(inst);
        tabu_search(inst);
    } else if(strcmp(inst->solver, "VNS") == 0) {
        greedy_iterative(inst);
        vns(inst);
    } else if(strcmp(inst->solver, "SIM_ANNEALING") == 0) {
        greedy_iterative(inst);
        simulated_annealing(inst);
    } else {
        print_error("Invalid solver selected");
    }
}

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

    inst->ncols = CPXgetnumcols(env, lp);

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
        CPXwriteprob(env, lp, "../data/model.lp", NULL);
}

int TSPopt(instance *inst){
    int error;
    //open cplex model
    CPXENVptr env = CPXopenCPLEX(&error);
    if(error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if(error) print_error("CPXcreateprob() error");

    build_model(inst, env, lp);

    set_params(inst, env);
    
    //track execution time
    time_t start, end;
    time(&start);

    int status = solve_problem(env, lp, inst);
    if(status){
        print_error("Unable to solve the problem");
    }

    time(&end);
    double elapsed = difftime(end, start);

    printf("Elapsed time : %f\n", elapsed);
    
    //Free the problem and close cplex environment
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return error;
}

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
    // Cplex output on screen
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	if (inst->verbose >= 10) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);

    if (inst->timelimit > 0) // Time limits <= 0 not allowed
        CPXsetdblparam(env, CPX_PARAM_TILIM, inst->timelimit);
    
    if (inst->randomseed >= 0)
        CPXsetintparam(env, CPX_PARAM_RANDOMSEED, inst->randomseed);
    
    /*if (inst->num_threads > 0) {
        CPXsetintparam(env, CPXPARAM_Threads, inst->num_threads);
    }*/

    // Cplex precision
    CPXsetdblparam(env, CPX_PARAM_EPINT, 0.0);		
	CPXsetdblparam(env, CPX_PARAM_EPGAP, 1e-9);	 
	CPXsetdblparam(env, CPX_PARAM_EPRHS, 1e-9); 
}
