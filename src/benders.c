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


double patching_heuristic(instance* inst, int* succ, int* comp, int* ncomp){

	while(*ncomp > 1){
		double min_z = INFINITY;
		int a = -1;
		int b = -1;

		//find the best delta_cost
		for(int i=0; i<inst->nnodes; i++){
			for(int j=0; j<inst->nnodes; j++){
				if(comp[i] < comp[j]){
					double new_z = get_cost(i, succ[j], inst);
					double actual_z = get_cost(j, succ[i], inst);
					if(new_z-actual_z < min_z){
						min_z = new_z-actual_z;
						a = i;
						b = j;
					}
				}
			}
		}

		//update the comp array
		int node = succ[b];
		int comp_a = comp[a];
		while(node != b){
			comp[node] = comp_a;
			node = succ[node];
		}
		comp[b] = comp_a;
		*ncomp = (*ncomp)-1;

		//update the successors array
		int a_first = succ[a];
		succ[a] = succ[b];
		succ[b] = a_first;
		
	}
	//we apply 2-opt in any case (also if we have 1 component)
	double cost = alg_2opt(inst, succ);

	return cost;
}

int benders(instance* inst, CPXENVptr env, CPXLPptr lp){
	//to check
	compute_distances(inst);


    int error;

    int ncols = CPXgetnumcols(env, lp);

	//to store the solution
	double* xstar = (double*) calloc(ncols, sizeof(double));
	//vector with the current solutions
	int* succ = (int*) calloc(inst->nnodes, sizeof(int));
	//components vector
	int* comp = (int*) calloc(inst->nnodes, sizeof(int));

	int it = 0;
	double z;
	//lower and upper bound
	double lb = -INFINITY;//CPX_INFBOUND; //not thread safe
	double ub = INFINITY;

    //track execution time
    time_t start, end;
    time(&start);

	while(lb < 0.9999 * ub) {
		int ncomp = 0;

		//If we exceeded the time limit: stop
        time(&end);
		double elapsed = difftime(end, start);
        if(elapsed > inst->timelimit) {
            free(succ);
            free(comp);
			free(xstar);
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

		//update the lower bound
		if(CPXgetobjval(env, lp, &z)) print_error("CPXgetobjval() error\n");
		lb = (z > lb) ? z : lb;

		//build the solution 
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
		if(error)
			print_error("CPXgetx() error");
		build_sol(xstar, inst, succ, comp, &ncomp);

		if(ncomp > 1){
        	//add subtour elimination constraints
			error = add_sec(inst, env, lp, ncomp, comp, ncols, it);
        	if(error) print_error("Failed in adding SEC");

		        
			//patching heuristic
			double cost = patching_heuristic(inst, succ, comp, &ncomp);
			if(update_solution(cost, succ, inst) && inst->verbose >= 50){
				printf("Using Patching Heuristic. Best solution updated: %f\n\n\n", cost);
			}

        } else {
			update_solution(z, succ, inst);
		}		

		ub = inst->zbest;
			
		it++;
	}

    free(succ);
	free(comp);
    free(xstar);

	return error;
}