#include "hardfixing.h"

int hard_fixing(instance* inst, CPXENVptr env, CPXLPptr lp) {
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
        } else if(objval >= bestobj && prob > 0.6) {
            prob -= 0.01;
            printf(" ... New probability set to %2.2lf | time = %3.2lf\n", prob, elapsed_time);
        }

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
