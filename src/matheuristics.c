#include "matheuristics.h"

int hard_fixing(instance* inst, CPXENVptr env, CPXLPptr lp) {
    // track execution time
    time_t start, end;
    time(&start);

    // original time limit
    double time_limit = inst->timelimit;
    // set fraction of time limit for metaheuristic
    inst->timelimit = time_limit / 10;

    // start with a good incumbent
	// use metaheuristic
    greedy_2opt(inst, 0);
    vns(inst);

    // set remaining time limit
    inst->timelimit = time_limit - time_limit / 10;
    
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

    // fixing params
    double prob = 0.8;
    char* lb = (char*) calloc(inst->nnodes, sizeof(char));
    double* zerobounds = (double*) calloc(inst->nnodes, sizeof(double));
    double* onebounds = (double*) calloc(inst->nnodes, sizeof(double));
    
    for(int i=0; i<inst->nnodes; ++i) {
        lb[i] = 'L';
        zerobounds[i] = 0.0;
        onebounds[i] = 1.0;
    }

    while(1) {
        time(&end);
        double elapsed_time = difftime(end, start);
        if(elapsed_time > inst->timelimit)
            break;

        // internal time limit for CPXmipopt
        double tilim = dmin(inst->timelimit - elapsed_time, inst->timelimit / 10.0);
        CPXsetdblparam(env, CPX_PARAM_TILIM, tilim);

        // select variables to be fixed
        int* indices = (int*) calloc(inst->nnodes, sizeof(int));
        int cnt = 0;

        for(int i=0; i<ncols; ++i) {
            double randnum = (double) rand() / RAND_MAX;

            if(xcurr[i] > 0.5 && randnum <= prob) {
                indices[cnt++] = i;
            }
        }

        // fix variables
        if(CPXchgbds(env, lp, cnt, indices, lb, onebounds))
            print_error("CPXchgbds() error");

        CPXwriteprob(env, lp, "fixmodel.lp", NULL);

        CPXmipopt(env, lp);

        // get new solution
        double objval;
        CPXgetx(env, lp, xcurr, 0, ncols-1);
        CPXgetobjval(env, lp, &objval);

        //printf(" ... Current solution = %f | prob = %f | time = %f\n", objval, prob, elapsed_time);

        if(objval < bestobj) {
            printf(" ... New solution found = %10.2lf | time = %3.2lf\n", objval, elapsed_time);

            bestobj = objval;
            memcpy(bestx, xcurr, ncols * sizeof(double));
        } else if(objval >= bestobj && prob > 0.5) {
            prob -= 0.01;
            printf(" ... New probability set to %2.2lf | time = %3.2lf\n", prob, elapsed_time);
        }/* else {
            prob = 0.5 + (rand() / (RAND_MAX / (0.8 - 0.5)));
            printf(" ... New probability set to %2.2lf | time = %3.2lf\n", prob, elapsed_time);
        }*/

        // unfix variables
        if(CPXchgbds(env, lp, cnt, indices, lb, zerobounds))
            print_error("CPXchgbds() error");

        CPXwriteprob(env, lp, "unfixmodel.lp", NULL);

        free(indices);
    }

    // save best solution found
    int* succ = (int*) calloc(inst->nnodes, sizeof(int));
    int* comp = (int*) calloc(inst->nnodes, sizeof(int));
    int ncomp;
    build_sol(bestx, inst, succ, comp, &ncomp);

    update_solution(bestobj, succ, inst);

    free(comp);
    free(succ);
    free(onebounds);
    free(zerobounds);
    free(lb);
    free(bestx);
    free(xcurr);
    free(index);
    return 0;
}

int local_branching(instance* inst, CPXENVptr env, CPXLPptr lp) {
    // track execution time
    time_t start, end;
    time(&start);

    // original time limit
    double time_limit = inst->timelimit;
    // set fraction of time limit for metaheuristic
    inst->timelimit = time_limit / 10;

    // start with a good incumbent
	// use metaheuristic
    greedy_2opt(inst, 0);
    vns(inst);

    // set remaining time limit
    inst->timelimit = time_limit - time_limit / 10;
    
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
	char** cname = (char**) calloc(1, sizeof(char*));		// (char **) required by cplex... array of 1 pointer
	cname[0] = (char*) calloc(100, sizeof(char));
	sprintf(cname[0], "local_branch");

    while(1) {
        time(&end);
        double elapsed_time = difftime(end, start);
		//printf("elapsed time: %f, timelimit: %f\n\n", elapsed_time, inst->timelimit);
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
		// double rhs = inst->nnodes - K;
		// in case change all the edges
		double rhs = inst->nnodes - dmin(K, inst->nnodes);

        int status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, indexes, values, NULL, &cname[0]);
		if (status)
			print_error("CPXaddrows() error");

		// CPXwriteprob(env, lp, "localbranch.lp", NULL);

        CPXmipopt(env, lp);

        // get new solution
        double objval;
        CPXgetx(env, lp, xcurr, 0, ncols-1);
        CPXgetobjval(env, lp, &objval);

        printf(" ... Current solution = %f | K = %d | time = %f\n", objval, K, elapsed_time);

		//update best solution if needed
		if(inst->zbest == -1 || objval < bestobj) {
			printf(" ... New solution found = %10.2lf | time = %3.2lf\n", objval, elapsed_time);
            bestobj = objval;
			memcpy(bestx, xcurr, ncols * sizeof(double));
			K += 10;
        } 
        
        int nrows = CPXgetnumrows(env, lp);

		// remove local branching constraint (last row)
		if(CPXdelrows(env, lp, nrows-1, nrows-1))
			print_error("CPXdelrows() error");

		// CPXwriteprob(env, lp, "freelocalb.lp", NULL);
    }

    // save best solution found
    int* succ = (int*) calloc(inst->nnodes, sizeof(int));
    int* comp = (int*) calloc(inst->nnodes, sizeof(int));
    int ncomp;
    build_sol(bestx, inst, succ, comp, &ncomp);

    update_solution(bestobj, succ, inst);

    free(comp);
    free(succ);
    
	free(cname[0]);
	free(cname);
	free(indexes);
    free(values);
    free(bestx);
    free(xcurr);
    free(index);
    return 0;
}