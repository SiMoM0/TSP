// SOLVER FILE

#pragma once

#include "utils.h"
#include "heuristics.h"
#include "metaheuristics.h"
#include "tspCplex.h"
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


