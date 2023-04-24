// SOLVER FILE

#pragma once

#include "utils.h"
#include <cplex.h>

/**
 * Solve instance of the model
 * 
 * @param inst model instance
*/
void solve_instance(instance* inst);

/**
 * Solve instance of the model using heuristic method
 * 
 * @param inst model instance
*/
void solve_heuristic(instance* inst);

/**
 * Solve instance of the model using cplex
 * 
 * @param inst model instance
*/
void solve_cplex(instance* inst);

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
