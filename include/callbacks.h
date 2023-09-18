// CALLBACKS FILE

#pragma once

#include "utils.h"
#include "solver.h"
#include <cplex.h>

// Data structure for Concorde relaxation callback function
typedef struct {
    CPXCALLBACKCONTEXTptr context;
    instance* inst;
    double* xstar;
} cc_params;


/**
 * Standard callback
 * 
 * @param context Cplex context
 * @param contextid context id
 * @param userhandle additional data
 * @return status
*/
int CPXPUBLIC sec_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* userhandle);

/**
 * Candidate callback
 * 
 * @param context Cplex context
 * @param contextid context id
 * @param inst model instance
 * @return status
*/
int candidate_callback(CPXCALLBACKCONTEXTptr context, CPXLONG conxtetid, instance* inst);

/**
 * Relaxation callback
 * 
 * @param context Cplex context
 * @param contextid context id
 * @param inst model instance
 * @return status
*/
int relaxation_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, instance* inst);

/**
 * Add relaxation cut
 * 
 * @param cutval value of the cut
 * @param nnodes number of nodes in the cut
 * @param cut vector that specify the indexes of the nodes in the cut
 * @param params user-handle parameters
 * @return status
*/
int relaxation_cut(double cutval, int cutcount, int* cut, void* params);

/**
 * Perform branch & cut algorithm through callbacks
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @return status
*/
int branch_and_cut(instance* inst, CPXENVptr env, CPXLPptr lp, CPXLONG contextid);

/**
 * Check cut
 * @param xstar current solution in cplex format
 * @param nnz number of non zero variables
 * @param rhs right hand side
 * @param sense constraint sense
 * @param index array containing indices of variables involved
 * @param value array with variables values
 * @return violation value (double)
*/
double cut_violation(double* xstar, int nnz, double rhs, char sense, int* index, double* value);