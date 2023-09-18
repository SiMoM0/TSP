// HARDFIXING

#pragma once

#include "utils.h"
#include "solver.h"
#include <cplex.h>

/**
 * Hard fixing matheuristic
 * 
 * @param inst model instance
 * @param env cplex environmental parameters
 * @param lp cplex lp data
 * @return status
*/
int hard_fixing(instance* inst, CPXENVptr env, CPXLPptr lp);