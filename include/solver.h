// SOLVER FILE

#pragma once

#include "utils.h"
#include "heuristics.h"
#include "metaheuristics.h"
#include "benders.h"


#include <cplex.h>

void solve(instance* inst);
/**
 * Solve instance of the model
 * 
 * @param inst model instance
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
*/
int TSPopt(instance* inst);

void build_sol(const double *xstar, instance *inst, int * succ, int* comp, int* ncomp);
void set_params(instance* inst, CPXENVptr env);