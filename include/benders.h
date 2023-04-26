#pragma once

#include "utils.h"
#include "solver.h"
#include <sys/time.h>
#include "plot.h"
#include <cplex.h>

int add_sec(instance *inst, CPXENVptr env, CPXLPptr lp, int ncomp, int *comp, int ncols, int it);

int Benders(instance* inst, CPXENVptr env, CPXLPptr lp);
