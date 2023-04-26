#include "benders.h"


/*int add_sec_videolezione_sbagliato(instance *inst, CPXENVptr env, CPXLPptr lp){
    int error;
    int ncols = CPXgetnumcols(env, lp);
    int ncomp = 0; //number of connected components

    int n_sec = 0;
    
    // keep track of the time
    time_t start_time = time(NULL);
    while (ncomp != 1 || difftime(time(NULL), start_time) < 100)
    {
        error = CPXmipopt(env, lp);
        if (error)
            print_error("CPXmipopt() error");
        ncols = CPXgetnumcols(env, lp);
        
        //allocating memory
        double *xstar = (double *)calloc(ncols, sizeof(double));
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
        if (error)
            print_error("CPXgetx() error");
        int *succ = (int *)malloc(sizeof(int) * inst->nnodes);
        int *comp = (int *)malloc(sizeof(int) * inst->nnodes);

        build_sol(xstar, inst, succ, comp, &ncomp);

        if (ncomp == 1) break; //there is only one cycle
        
        //cc stands for connected components
        for (int cc = 1; cc <= ncomp; cc++)
        {
            char **cname = (char **)calloc(1, sizeof(char *));
            cname[0] = (char *)calloc(256, sizeof(char));
            sprintf(cname[0], "sec(%d)", n_sec);
            
            int nncc = 0; //number of connected components
            for (int i = 0; i < inst->nnodes; i++)
            {
                if (comp[i] == cc)
                    nncc++;
            }

            //TO DO forse l'allocazione di index e value va fuori dal for
            int *index = (int *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(int));
            double *value = (double *)calloc((inst->nnodes * (inst->nnodes - 1)) / 2, sizeof(double));
            char sense = 'L'; // 'L' for less than or equal to constraint
            int nnz = 0;
            int izero = 0;
            double rsh = nncc - 1; //NON NE SONO SICURA


            int j = 0;
            int k;
            while (j < inst->nnodes - 1)
            {
                while (comp[j] != cc && j < inst->nnodes)
                {
                    j++;
                }
                k = j + 1;
                while (k < inst->nnodes)
                {
                    if (comp[k] == cc)
                    {
                        index[nnz] = xpos(j, k, inst);
                        value[nnz] = 1.0;
                        nnz++;
                    }
                    k++;
                }
                j++;
            }

            
            if (CPXaddrows(env, lp, 0, 1, nnz, &rsh, &sense, &izero, index, value, NULL, &cname))
                print_error("CPXaddrows(): error 1");

            free(cname[0]);
            free(cname);
            free(index);
            free(value);
            CPXwriteprob(env, lp, "mipopt.lp", NULL);
        }
        build_sol(xstar, inst, succ, comp, &ncomp);
        free(xstar);
        free(succ);
        free(comp);
    }
    if(ncomp == 1)
        return 1; //single cycle
    else
        return 0;
} 
*/
int add_sec(instance *inst, CPXENVptr env, CPXLPptr lp, int ncomp, int *comp, int ncols, int it){
    int* index = (int*)malloc(ncols * sizeof(int));
	double* value = (double*)malloc(ncols * sizeof(double));
	

	//colname parameter for CPXnewcols
	char** cname = (char**)calloc(1, sizeof(char*));		// (char **) required by cplex... array of 1 pointer
	cname[0] = (char*)calloc(100, sizeof(char));

	//for each component
	for (int k = 1; k <= ncomp; k++) {
		char sense = 'L'; // <= constraint
		double num_nodes = 0.0;
		int nnz = 0;  //Number of variables to add in the constraint
		int izero = 0;
		

		sprintf(cname[0], "sec(%d,%d)",it,k);

		for (int i = 0; i < inst->nnodes; i++) {
			if (comp[i] != k) continue;
			num_nodes++;
			
			for (int j = i + 1; j < inst->nnodes; j++) {
				if (comp[j] != k) continue;

				index[nnz] = xpos(i, j, inst);
				value[nnz] = 1.0;
				nnz++;
			}
		}


		double rhs = num_nodes - 1;	// |S|-1

		if(CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, cname)) print_error("CPXaddrows(): error 1");
		
	}

	
	
	if (inst->verbose >= 1) {
		CPXwriteprob(env, lp, "modelSEC.lp", NULL);
	}

	free(value);
	free(index);

	free(cname[0]);
	free(cname);
}


int Benders(instance* inst, CPXENVptr env, CPXLPptr lp){
   
    int error;
    printf("--- starting Benders ---\n");

    int ncols = CPXgetnumcols(env, lp);

	//to store the solution
	double* xstar = (double*)calloc(ncols, sizeof(double));

	//vector with the current solutions
	int* succ = (int*)malloc(inst->nnodes * sizeof(int));

	//components vector
	int* comp = (int*)malloc(inst->nnodes * sizeof(int));
	int ncomp = 0;

	int it = 0;

    //count the time
    struct timeval start, end;
    gettimeofday(&start, 0);

	do {
        
        //Non so se va messa qua
		ncomp = 0;

		//If we exceeded the time limit: stop
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        double timelimit = (double) inst->timelimit;
        if (timelimit > 0 && elapsed > timelimit) {
            free(succ);
            free(comp);
            //return CPX_STAT_ABORT_TIME_LIM;

            //TO FIX
            printf("\n\n\n\n\ntempo scaduto\n\n\n\n");
            return 0;
        }

    
        // We apply to cplex the residual time left to solve the problem
        if (timelimit > 0 && elapsed < timelimit) {
            double new_timelim = timelimit - elapsed; // The residual time limit that is left
            CPXsetdblparam(env, CPXPARAM_TimeLimit, new_timelim);
            double here = 0;
            CPXgetdblparam(env, CPXPARAM_TimeLimit, &here);
        }
    

		// run CPLEX to get a solution
		error = CPXmipopt(env, lp);
		if (error)
		{
			printf("CPX error code %d\n", error);
			print_error("CPXmipopt() error");
		}


		// use the optimal solution found by CPLEX and prints it
        error = CPXgetx(env, lp, xstar, 0, ncols - 1);
		if (error) {
			if (inst->zbest != -1) {
				printf("----- Terminated before convergence -----\n");
				printf("BEST SOLUTION FOUND\nCOST: %f\n", inst->zbest);
                //plot solution 
                //DA AGGIUNGERE LA FUNZIONE
				return 0;
			}
			else print_error("CPXgetx() error");
		}
		for (int i = 0; i < inst->nnodes; i++)
		{
			for (int j = i + 1; j < inst->nnodes; j++)
			{
				if (xstar[xpos(i, j, inst)] > 0.5) printf("  ... x(%3d,%3d) = 1\n", i + 1, j + 1);
			}
		}


		// ----- Transforms xstar to succ and comp -----
		build_sol(xstar, inst, succ, comp, &ncomp);

		if(inst->verbose >= 50) printf("ncomp = %d\n", ncomp);

		if (ncomp == 1){
            break;
            printf("ncomp = 1\n");
        } 

		//add subtour elimination constraints
		add_sec(inst, env, lp, ncomp, comp, ncols, it);
        
		//repair current solution
		//if(refinement(inst, succ, comp, ncomp, it)) return 1;

		it++;

	} while (1);

    //while (ncomp>1);

    double z;
    error = CPXgetobjval(env, lp, &z);
    if(error)
        printf("CPXgetobjval() error\n");

    update_solution(z, succ, inst);

	free(succ);
	free(comp);
    free(xstar);
	return 0;
}