#include "callbacks.h"

int CPXPUBLIC sec_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* userhandle) { 
	instance* inst = (instance*) userhandle;

	if(contextid == CPX_CALLBACKCONTEXT_CANDIDATE) {
		return candidate_callback(context, contextid, inst);
	}
	if(contextid = CPX_CALLBACKCONTEXT_RELAXATION) {
		return relaxation_callback(context, contextid, inst);
	}
	
	return 1;
}

int candidate_callback(CPXCALLBACKCONTEXTptr context, CPXLONG conxtetid, instance* inst) {
	double* xstar = (double*) malloc(inst->ncols * sizeof(double));
	double objval = CPX_INFBOUND;
	if(CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval))
        print_error("CPXcallbackgetcandidatepoint error");
	
	// get some random information at the node
	int mythread = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread); 
	int mynode = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode); 
	double incumbent = CPX_INFBOUND; CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent); 
	
    if(inst->verbose >= 100)
        printf(" ... callback at node %5d thread %2d incumbent %10.2lf, candidate value %10.2lf\n", mynode, mythread, incumbent, objval);
	
    int* succ = (int*) calloc(inst->nnodes, sizeof(int));
    int* comp = (int*) calloc(inst->nnodes, sizeof(int));
    int ncomp;

    build_sol(xstar, inst, succ, comp, &ncomp);

	//... if xstart is infeasible, find a violated cut and store it in the usual Cplex's data structute (rhs, sense, nnz, index and value)
	
	if(ncomp > 1) // means that the solution is infeasible and a violated cut has been found
	{
        int* index = (int*) calloc(inst->ncols, sizeof(int));
        double* value = (double*) calloc(inst->ncols, sizeof(double));
		
        //for each component
    	for(int k = 1; k <= ncomp; k++) {
    		char sense = 'L'; // <= constraint
    		double num_nodes = 0.0;
    		int nnz = 0;  //Number of variables to add in the constraint
    		int izero = 0;

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

    		if(CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value))
                print_error("CPXcallbackrejectcandidate() error"); // reject the solution and adds one cut 
            //if ( CPXcallbackrejectcandidate(context, 0, NULL, NULL, NULL, NULL, NULL, NULL) ) print_error("CPXcallbackrejectcandidate() error"); // just reject the solution without adding cuts (less effective)
    	}

		free(value);
		free(index);	
	}
	
    free(comp);
    free(succ);
	free(xstar); 
	return 0;
}

int relaxation_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance* inst) {
	
}

int branch_and_cut(instance* inst, CPXENVptr env, CPXLPptr lp, CPXLONG contextid) {
    // install a "lazyconstraint" callback to cut infeasible integer sol.s (found e.g. by heuristics) 
	//CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE; // ... means lazyconstraints
	if(CPXcallbacksetfunc(env, lp, contextid, sec_callback, inst))
        print_error("CPXcallbacksetfunc() error");
	
	//CPXsetintparam(env, CPX_PARAM_THREADS, 1); 	// just for debugging

	CPXmipopt(env,lp); 		// with the callback installed
    
    double* xstar = (double*) calloc(inst->ncols, sizeof(double));
    CPXgetx(env, lp, xstar, 0, inst->ncols-1);

    int* succ = (int*) calloc(inst->nnodes, sizeof(int));
    int* comp = (int*) calloc(inst->nnodes, sizeof(int));
    int ncomp;

    build_sol(xstar, inst, succ, comp, &ncomp);

    double z;
    int error = CPXgetobjval(env, lp, &z);
    if(error)
        print_error("CPXgetobjval() error\n");

    update_solution(z, succ, inst);

    free(comp);
    free(succ);
    free(xstar);

    return 0;
}
