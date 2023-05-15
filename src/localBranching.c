#include "localBranching.h"

int local_branching(instance *inst, CPXENVptr env, CPXLPptr lp) {
    //track execution time
    time_t start, end;
    time(&start);

    //define variables
    int ncols = CPXgetnumcols(env, lp);
    int* index = (int*) calloc(ncols, sizeof(int));
    int K = 20;
    int it = 0;

    // start with a good incumbent
    printf("COMPUTE HEURISTIC SOLUTION\n");
    greedy_2opt(inst, 0);

    double objbest = inst->zbest;
    

    //setting callback funct
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE; //| CPX_CALLBACKCONTEXT_RELAXATION;
    int status = CPXcallbacksetfunc(env, lp, contextid, sec_callback, inst);
    if (status) {
        printf("CPXcallbacksetfunc() error returned status %d", status);
    }

    // build degree constraints for cplex
    double *values = (double*) calloc(ncols, sizeof(double));
    int* indices = (int*) calloc(inst->nnodes, sizeof(int));
    char *names = (char*) calloc(100, sizeof(char));
    double* xcurr = (double*) calloc(ncols, sizeof(double)); //it is xheu


    int u = 0;
    for(int i=0; i<inst->nnodes; ++i) {
        for(int j=i+1; j<inst->nnodes; ++j) {
            index[u++] = xpos(i, j, inst);
        }
    }
    
    printf("BUILT DEGREE CONSTRAINTS\n");


    int beg = 0;
    int effortlevel = CPX_MIPSTART_NOCHECK;
    if(CPXaddmipstarts(env, lp, 1, ncols, &beg, index, xcurr, &effortlevel, NULL))
        print_error("CPXaddmipstarts() error");

    printf("ADD MIP START\n");

    while(1) {

        //keep track of the time
        time(&end);
        double elapsed_time = difftime(end, start);
        if(elapsed_time > inst->timelimit)
            break;

        double tilim = dmin(inst->timelimit - elapsed_time, inst->timelimit / 10.0);
        printf("TILIM = %f\n", tilim);
        CPXsetdblparam(env, CPX_PARAM_TILIM, tilim);

        // Add new constraints
        double rhs = inst->nnodes-K;
        char sense = 'G';   // >=
        int nnz = 0;

        //change from succ to CPLEX
        bzero(xcurr, inst->ncols*sizeof(double));
        for(int i=0; i<inst->nnodes; ++i) {
            xcurr[xpos(i, inst->best_sol[i], inst)] = 1.0;
        }

       
        for (int i = 0; i < ncols; i++) {
            if (xcurr[i] > 0.5) {
                index[nnz] = i;
                values[nnz] = 1.0;
                nnz++;
            }
        }
        int izero = 0;
        int status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, values, NULL, &names); 
        if (status) {
            printf("CPXaddrows in softfixing error code %d", status);
        }

        // Solve the model
        status = CPXmipopt(env, lp);
        if (status) {
            printf("CPXmipopt error code %d", status);
        }

        //Retreive the solution
        status = CPXgetx(env, lp, xcurr, 0, ncols - 1);
        double objval;
        CPXgetobjval(env, lp, &objval);
        if (status) { 
            printf("CPXgetx error code %d", status); 
        }
        printf("objval = %f, objbest = %f\n\n", objval, objbest);
        if (objval < objbest) {
            K += 10;
            //update_solution(objval, xcurr, inst);
        }

        // Remove the added soft-fixing constraints
        int numrows = CPXgetnumrows(env, lp);
        status = CPXdelrows(env, lp, numrows - 1, numrows - 1);
        if (status) {
            printf("CPXdelrows error code %d", status);
        }
        
        it++;
        
    }

    free(indices);
    free(names);
    free(xcurr);
    free(index);
    free(values);
    return 0;
}