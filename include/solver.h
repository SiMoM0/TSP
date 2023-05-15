// SOLVER FILE

#pragma once

#include "utils.h"
#include "heuristics.h"
#include "metaheuristics.h"
#include "benders.h"
#include "callbacks.h"
#include "localBranching.h"

#include <cplex.h>

/**
 * Solve instance of the model
 * 
 * @param inst model instance
*/
void solve(instance* inst);

/**
 * Solve instance using Cplex
 * 
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @param inst model instance
 * @return status
*/
int solve_problem(CPXENVptr env, CPXLPptr lp, instance *inst);

/**
 * Solve instance of the model using heuristic method
 * 
 * @param inst model instance
*/
void solve_heuristic(instance* inst);

/**
 * Build instance model in CPLEX
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
*/
void build_model(instance* isnt, CPXENVptr env, CPXLPptr lp);

/**
 * Optimize TSP instance using cplex
 * 
 * @param inst model instance
 * @return status
*/
int TSPopt(instance* inst);

/**
 * Build TSP solution after Cplex optimization.
 * Solution in represented by the successor vector.
 * 
 * @param xstar Cplex solution
 * @param inst model instance
 * @param succ successor vector to store solution
 * @param comp component vector
 * @param ncomps number of components
*/
void build_sol(const double *xstar, instance *inst, int * succ, int* comp, int* ncomp);

/**
 * Set CPLEX parameters
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
*/
void set_params(instance* inst, CPXENVptr env);