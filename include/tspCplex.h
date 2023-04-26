#pragma once

#include "utils.h"
#include "solver.h"

#include <cplex.h>
#include <sys/time.h>






void build_model(instance* inst, CPXENVptr env, CPXLPptr lp);
void build_sol(const double *xstar, instance *inst, int * succ, int* comp, int* ncomp);
void set_params(instance* inst, CPXENVptr env);
int TSPopt(instance *inst);