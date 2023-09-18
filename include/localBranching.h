#pragma once

#include "utils.h"
#include "solver.h"
#include <cplex.h>

/**
 * Local Branching algorithm
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @return status
*/
int local_branching(instance *inst, CPXENVptr env, CPXLPptr lp);
