#pragma once

#include "utils.h"
#include "solver.h"
#include <cplex.h>

/**
 * Add subtour elimination constraints (SEC)
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @param ncomp number of components
 * @param comp vector representing the components
 * @param ncols number of columns
 * @param it
 * @return status
*/
int add_sec(instance *inst, CPXENVptr env, CPXLPptr lp, int ncomp, int *comp, int ncols, int it);

/**
 * Patching heuristic
 * 
 * @param inst model instance
 * @param succ successor vector
 * @param comp vector representing the components
 * @param ncomp number of components
 * @return cost
*/
double patching_heuristic(instance* inst, int* succ, int* comp, int* ncomp);

/**
 * Perform Bender's loop
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @return status
*/
int benders(instance* inst, CPXENVptr env, CPXLPptr lp);
