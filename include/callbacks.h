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
int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* userhandle);

int branch_and_cut(instance* inst, CPXENVptr env, CPXLPptr lp);