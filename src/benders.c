#include "benders.h"


int add_sec(instance *inst, CPXENVptr env, CPXLPptr lp, int ncomp, int *comp, int ncols, int it){
    int* index = (int*) malloc(ncols * sizeof(int));
	double* value = (double*) malloc(ncols * sizeof(double));
	
	//colname parameter for CPXnewcols
	char** cname = (char**) calloc(1, sizeof(char*));		// (char **) required by cplex... array of 1 pointer
	cname[0] = (char*) calloc(100, sizeof(char));

	//for each component
	for(int k = 1; k <= ncomp; k++) {
		char sense = 'L'; // <= constraint
		double num_nodes = 0.0;
		int nnz = 0;  //Number of variables to add in the constraint
		int izero = 0;

		sprintf(cname[0], "sec(%d,%d)",it,k);

		for(int i = 0; i < inst->nnodes; i++) {
			if(comp[i] != k) continue;
			
			num_nodes++;
			
			for(int j = i + 1; j < inst->nnodes; j++) {
				if(comp[j] != k) continue;

				index[nnz] = xpos(i, j, inst);
				value[nnz] = 1.0;
				nnz++;
			}
		}

		double rhs = num_nodes - 1;	// |S|-1

		if(CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, cname))
			print_error("CPXaddrows(): error 1");
	}

	if (inst->verbose >= 100) {
		CPXwriteprob(env, lp, "../data/modelSEC.lp", NULL);
	}

	free(value);
	free(index);
	free(cname[0]);
	free(cname);
}


int benders(instance* inst, CPXENVptr env, CPXLPptr lp){
    int error;

    int ncols = CPXgetnumcols(env, lp);

	//to store the solution
	double* xstar = (double*) calloc(ncols, sizeof(double));
	//vector with the current solutions
	int* succ = (int*) calloc(inst->nnodes, sizeof(int));
	//components vector
	int* comp = (int*) calloc(inst->nnodes, sizeof(int));

	int it = 0;

    //track execution time
    time_t start, end;
    time(&start);

	while(1) {
		int ncomp = 1;

		//If we exceeded the time limit: stop
        time(&end);
		double elapsed = difftime(end, start);
        if(elapsed > inst->timelimit) {
            free(succ);
            free(comp);
            return 2;
        }

        // apply to cplex the residual time left to solve the problem
        if (inst->timelimit > 0 && elapsed < inst->timelimit) {
            double new_timelim = inst->timelimit - elapsed; // The residual time limit that is left
            CPXsetdblparam(env, CPX_PARAM_TILIM, new_timelim);
        }
    
		// run CPLEX to get a solution
		error = CPXmipopt(env, lp);
		if (error) {
			printf("CPX error code %d\n", error);
			print_error("CPXmipopt() error");
		}


		// use the optimal solution found by CPLEX and prints it
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
		if(error) {
			if (inst->zbest != -1) {
				printf("----- Terminated before convergence -----\n");
				printf("BEST SOLUTION FOUND\nCOST: %f\n", inst->zbest);
				return error;
			} else {
				print_error("CPXgetx() error");
			}
		}

		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = i + 1; j < inst->nnodes; j++) {
				if(xstar[xpos(i, j, inst)] > 0.5)
					printf("  ... x(%3d,%3d) = 1\n", i + 1, j + 1);
			}
		}

		build_sol(xstar, inst, succ, comp, &ncomp);

		if(inst->verbose >= 50)
			printf("Current number of components = %d\n", ncomp);

		if(ncomp == 1){
            break;
        } 

		//add subtour elimination constraints
		error = add_sec(inst, env, lp, ncomp, comp, ncols, it);
        if(error) print_error("FAILED in adding SEC");
        
		//repair current solution
		//if(refinement(inst, succ, comp, ncomp, it)) return 1;

		it++;
	}

    double z;
    error = CPXgetobjval(env, lp, &z);
    if(error)
        print_error("CPXgetobjval() error\n");

    update_solution(z, succ, inst);

	free(succ);
	free(comp);
    free(xstar);

	return error;
}