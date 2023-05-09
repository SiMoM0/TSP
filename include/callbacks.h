// CALLBACKS FILE

#pragma once

#include "utils.h"
#include "solver.h"
#include <cplex.h>

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
 * Perform branch & cut algorithm through callbacks
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @return status
*/
int branch_and_cut(instance* inst, CPXENVptr env, CPXLPptr lp, CPXLONG contextid);