#include "localBranching.h"


/*int local_branching(instance* inst, CPXENVptr env, CPXLPptr lp) {
    time_t start, end;
    time(&start);

	
    printf("COMPUTE HEURISTIC SOLUTION\n");
    greedy_2opt(inst, 0);
	//greedy(inst,0);
    double bestobj = inst->zbest;

	int ncols = CPXgetnumcols(env, lp);
	inst->ncols = ncols;

    
	double* xcurr = (double*)calloc(ncols, sizeof(double));
    

	CPXsetdblparam(env, CPX_PARAM_TILIM, inst->timelimit / 2);

	int* ind = (int*)malloc(inst->ncols * sizeof(int));
	for (int j = 0; j < inst->ncols; j++){
        ind[j] = j;
    }
        
    for (int i = 0; i < inst->nnodes; i++){
        xcurr[xpos(i, inst->best_sol[i], inst)] = 1.0;
    } 


	CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE; //lazyconstraints
    int status = CPXcallbacksetfunc(env, lp, contextid, sec_callback, inst);
	if (status)
        print_error("CPXcallbacksetfunc() error");
    
	
	int effortlevel = CPX_MIPSTART_NOCHECK;
	int beg = 0;

    status = CPXaddmipstarts(env, lp, 1, inst->ncols, &beg, ind, xcurr, &effortlevel, NULL);
	if (status)
        print_error("CPXaddmipstarts() error");
    
	
	int* indexes = (int*)malloc(ncols * sizeof(int));
	double* values = (double*)malloc(ncols * sizeof(double));

	//define the array of radius
	int rad [] = {(int)inst->nnodes/20,(int)inst->nnodes/25,(int)inst->nnodes/30};
	int radindex = 0;
	int it = 0;


	char sense = 'G';   // >=
	int matbeg = 0;
	char* names = (char*)malloc(100*sizeof(char));

	int n_small_improvements = 0;

	while (1) {

        //keep track of the time
        time(&end);
        double elapsed_time = difftime(end, start);
        //printf("elapsed: %f, timelimit:%f\n\n\n\n", elapsed_time, inst->timelimit);
        if(elapsed_time > inst->timelimit){
            printf("Time out\n");
            break;
        }   

        // Add new constraints according to the radius: SUM_{x_e=1}{x_e}>=n-radius
		int nnz = 0;
		for (int i = 0; i < ncols; i++)
		{
			if (xcurr[i] > 0.5) {
				indexes[nnz] = i;
				values[nnz] = 1.0;
				nnz++;
			}
		}
        //printf("rad[radindex] = %d\n", rad[radindex]);
		double rhs = inst->nnodes - rad[radindex];
		//sprintf(names, "new_constraint(%d)", it++);

        status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, indexes, values, NULL, &names);
		if (status)
			print_error("CPXaddrows() error");
		
        status = CPXmipopt(env, lp);
		if (status)
		    print_error("CPXmipopt() error");
		

		int* succ = (int*)malloc(inst->nnodes * sizeof(int));
		int* comp = (int*)malloc(inst->nnodes * sizeof(int));
		int ncomp = 0;
        status = CPXgetx(env, lp, xcurr, 0, ncols - 1);
		if (status) 
            print_error("CPXgetx() error");
            
		build_sol(xcurr, inst, succ, comp, &ncomp);


		double objval = -1;
        status = CPXgetobjval(env, lp, &objval);
		printf("objval: %f, bestz: %f\n\n\n", objval, inst->zbest);
		if (status) 
            print_error("CPXgetobjval() error");

		//update best solution if needed
		if (ncomp == 1) {
			if (inst->zbest == -1 || inst->zbest > objval) {
				if (n_small_improvements  > 0){
					n_small_improvements --;
				}
                update_solution(objval, succ, inst);
				
			}
			else {
				n_small_improvements ++;
			}
			if (n_small_improvements  > 3)
				radindex++;
			if (radindex > 2)
				radindex = 0;
		}
		free(succ);
		free(comp);


		int numrows = CPXgetnumrows(env, lp);
        status = CPXdelrows(env, lp, numrows - 1, numrows - 1);
		if (status)
			print_error("CPXdelrows() error");
		
	} 
    free(ind);
	free(indexes);
	free(values);
	free(names);


	if (inst->verbose >= 1) printf("BEST SOLUTION FOUND\nCOST: %f  with greedy was: %f\n", inst->zbest, bestobj);

	free(xcurr);
	return 0;
}*/


int local_branching(instance* inst, CPXENVptr env, CPXLPptr lp) {
    // track execution time
    time_t start, end;
    time(&start);

    // start with a good incumbent
    greedy_2opt(inst, 0);
    
    if(CPXcallbacksetfunc(env, lp, CPX_CALLBACKCONTEXT_CANDIDATE, sec_callback, inst))
        print_error("CPXcallbacksetfunc() error");

    // build constraints for cplex
    int ncols = CPXgetnumcols(env, lp);
    int* index = (int*) calloc(ncols, sizeof(int));
    double* xcurr = (double*) calloc(ncols, sizeof(double));

    for(int i=0; i<ncols; ++i) {
        index[i] = i;
    }

    for(int i=0; i<inst->nnodes; ++i) {
        xcurr[xpos(i, inst->best_sol[i], inst)] = 1.0;
    }

    int beg = 0;
    int effortlevel = CPX_MIPSTART_NOCHECK;
    if(CPXaddmipstarts(env, lp, 1, ncols, &beg, index, xcurr, &effortlevel, NULL))
        print_error("CPXaddmipstarts() error");

    // best solution found
    double bestobj = inst->zbest;
    double* bestx = (double*) calloc(ncols, sizeof(double));
    memcpy(bestx, xcurr, ncols * sizeof(double));

	int K = 20;
	int* indexes = (int*)malloc(ncols * sizeof(int));
	double* values = (double*)malloc(ncols * sizeof(double));
	int it = 0;


	char sense = 'G';   // >=
	int matbeg = 0;
	char* names = (char*)malloc(100*sizeof(char));

    while(1) {
        time(&end);
        double elapsed_time = difftime(end, start);
		printf("elapsed time: %f, timelimit: %f\n\n", elapsed_time, inst->timelimit);
        if(elapsed_time > inst->timelimit)
            break;

        // internal time limit for CPXmipopt
        double tilim = dmin(inst->timelimit - elapsed_time, inst->timelimit / 10.0);
        CPXsetdblparam(env, CPX_PARAM_TILIM, tilim);

		// Add new constraints according to the radius: SUM_{x_e=1}{x_e}>=n-radius
		int nnz = 0;
		for (int i = 0; i < ncols; i++)
		{
			if (xcurr[i] > 0.5) {
				indexes[nnz] = i;
				values[nnz] = 1.0;
				nnz++;
			}
		}
        //printf("rad[radindex] = %d\n", rad[radindex]);
		double rhs = inst->nnodes - K;
		//rad[radindex];
		//sprintf(names, "new_constraint(%d)", it++);

        int status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, indexes, values, NULL, &names);
		if (status)
			print_error("CPXaddrows() error");

        CPXmipopt(env, lp);

		int* succ = (int*)malloc(inst->nnodes * sizeof(int));

		//components vector
		int* comp = (int*)malloc(inst->nnodes * sizeof(int));
		int ncomp = 0;
		if (CPXgetx(env, lp, xcurr, 0, ncols - 1)) print_error("error on CPXgetx");
		build_sol(xcurr, inst, succ, comp, &ncomp);

        // get new solution
        double objval;
        CPXgetx(env, lp, xcurr, 0, ncols-1);
        CPXgetobjval(env, lp, &objval);

        //printf(" ... Current solution = %f | prob = %f | time = %f\n", objval, prob, elapsed_time);

		//update best solution if needed
		
		if(inst->zbest == -1 || objval < bestobj) {
			printf(" ... New solution found = %10.2lf | time = %3.2lf\n", objval, elapsed_time);
            bestobj = objval;
			memcpy(bestx, xcurr, ncols * sizeof(double));
			K += 10;
        } 
        
    }

    // save best solution found
    int* succ = (int*) calloc(inst->nnodes, sizeof(int));
    int* comp = (int*) calloc(inst->nnodes, sizeof(int));
    int ncomp;
    build_sol(bestx, inst, succ, comp, &ncomp);

    update_solution(bestobj, succ, inst);

    free(comp);
    free(succ);
    
    
    free(bestx);
    free(xcurr);
    free(index);
    return 0;
}