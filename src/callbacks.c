#include "callbacks.h"

#include <concorde.h>
#include <cut.h>

int CPXPUBLIC sec_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* userhandle) { 
	instance* inst = (instance*) userhandle;

	if(contextid == CPX_CALLBACKCONTEXT_CANDIDATE) {
		return candidate_callback(context, contextid, inst);
	}
	if(contextid == CPX_CALLBACKCONTEXT_RELAXATION) {
		return relaxation_callback(context, contextid, inst);
	}

	print_error("contextid unknown in callback");
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
	
	if(ncomp > 1) {// means that the solution is infeasible and a violated cut has been found
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
	} else if(ncomp == 1) {
		// Posting heuristic
		double objheu = alg_2opt(inst, succ);

		// convert solution
		double* xheu = (double*) calloc(inst->ncols, sizeof(double));  // all zeros, initially
		for(int i=0; i<inst->nnodes; ++i)
			xheu[xpos(i,succ[i],inst)] = 1.0;

		int *ind = (int *) malloc(inst->ncols * sizeof(int));
		for(int i=0; i<inst->ncols; ++i) 
			ind[i] = i;

		if(CPXcallbackpostheursoln(context, inst->ncols, ind, xheu, objheu, CPXCALLBACKSOLUTION_NOCHECK))
			print_error("CPXcallbackpostheursoln() error");

		// printf(" ... Post heuristic solution with objective = %f\n", objheu);
		free(ind);
		free(xheu);
	}
	
    free(comp);
    free(succ);
	free(xstar); 
	return 0;
}

int relaxation_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance* inst) {
	double* xstar = (double*) calloc(inst->ncols, sizeof(double));
	double objval = CPX_INFBOUND;
	if(CPXcallbackgetrelaxationpoint(context, xstar, 0, inst->ncols-1, &objval))
        print_error("CPXcallbackgetcandidatepoint error");
	
	// get some random information at the node
	int mythread = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);
	int mynode = -1; CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &mynode);
	double incumbent = CPX_INFBOUND; CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
	
	// separation frequency
	if(mynode % inst->nnodes != 0)
		return 0;

	int* comps = (int*) calloc(inst->nnodes, sizeof(int));			// nodes pertaining to each component, array of length nnodes
	int* compscount = (int*) calloc(inst->nnodes, sizeof(int));		// number of nodes in each component, array of length ncomp
    int ncomp;														// number of connected components

	// edge list in concorde format
	int* elist = (int*) calloc((inst->ncols * 2), sizeof(int));
	int ecount = 0;

	// fill edge list
	int idx = 0;
	for(int i=0; i<inst->nnodes; ++i) {
		for(int j=i+1; j<inst->nnodes; ++j) {
			elist[idx++] = i;
			elist[idx++] = j;
			ecount++;
		}
	}

	if(CCcut_connect_components(inst->nnodes, ecount, elist, xstar, &ncomp, &compscount, &comps))
		print_error("CCcut_connect_components error");

	// printf("NCOMP = %d\n", ncomp);

	if(inst->verbose >= 100)
        	printf(" ... relaxation callback at node %5d thread %2d incumbent %10.2lf, candidate value %10.2lf, number of components %5d\n", mynode, mythread, incumbent, objval, ncomp);

	cc_params params;
        params.context = context;
        params.inst = inst;
		params.xstar = xstar;

	if(ncomp == 1) {
        if(CCcut_violated_cuts(inst->nnodes, ecount, elist, xstar, 1.9, relaxation_cut, &params))
            print_error("CCcut_violated_cuts error");
    } else if (ncomp > 1) {
		int start = 0;

		// add sec for each components
		for(int c=0; c<ncomp; ++c) {
			// number of nodes in the current component
			int compsize = compscount[c];

			//printf("Component #%d of %d | size = %d\n", c, ncomp, compsize);

			// current subtour in the graph
			int* subtour = (int*) calloc(compsize, sizeof(int));
			
			for(int i=0; i<compsize; ++i) {
				subtour[i] = comps[i+start];
				//printf("subtour[%d] = %d\n", i, subtour[i]);
			}

			double cutval = 0.0; // default value to pass checks
			relaxation_cut(cutval, compsize, subtour, &params);

			//printf(" ... add usercut with comp #%d of %d\n", c, ncomp);

			start += compsize;

			free(subtour);
		}
	}

	free(elist);
	free(compscount);
	free(comps);
	free(xstar);
	return 0;
}

int relaxation_cut(double cutval, int cutnodes, int* cut, void* params) {
	cc_params* param = (cc_params*) params;

	int ecount = cutnodes * (cutnodes - 1) / 2;
	int* index = (int*) calloc(ecount, sizeof(int));
    double* value = (double*) calloc(ecount, sizeof(double));

	char sense = 'L'; // <= constraint
	double rhs = cutnodes - 1;
	int purgeable = CPX_USECUT_FILTER;
	int matbeg = 0;
	int local = 0;

	int nnz = 0;
    for (int i=0; i<cutnodes; ++i) {
        for (int j=i+1; j<cutnodes; ++j) {
            index[nnz] = xpos(cut[i], cut[j], param->inst);
            value[nnz] = 1.0;
			nnz++;
        }
    }

	// printf("RHS = %f | nnz = %d\n", rhs, nnz);

	// check cut violation
	double violation = cut_violation(param->xstar, nnz, rhs, sense, index, value);
	// printf(" violation = %f | rhs = %f | nnz = %d | cutval = %f\n", violation, rhs, nnz, cutval);

	if(fabs(violation - (2.0 - cutval) / 2.0) > EPSILON)
		print_error("Inconsistent violation");

    if(CPXcallbackaddusercuts(param->context, 1, nnz, &rhs, &sense, &matbeg, index, value, &purgeable, &local))
        print_error("Error on CPXcallbackaddusercuts()");

	// printf(" ... Added usercuts\n");
	free(value);
	free(index);
	return 0;
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

double cut_violation(double* xstar, int nnz, double rhs, char sense, int* index, double* value) {
	double lhs = 0.0;

	for(int i=0; i<nnz; ++i) {
		lhs += xstar[index[i]] * value[i];
	}

	if(sense == 'L')
		return lhs - rhs;
	else if(sense == 'G')
		return rhs - lhs;
	else
		return fabs(lhs - rhs);
}
