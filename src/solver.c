#include "solver.h"
#include "heuristics.h"
#include "metaheuristics.h"

void solve_instance(instance* inst) {
    if(strncmp(inst->solver, "CPLEX", 5) == 0) {
        solve_cplex(inst);
    } else {
        solve_heuristic(inst);
    }
}

void solve_heuristic(instance* inst) {
    if(strcmp(inst->solver, "GREEDY") == 0) {
        //TODO how to select starting node
        greedy(inst, 0);
    } else if(strcmp(inst->solver, "GRASP") == 0) {
        grasp(inst, 0, 0.8);
    } else if(strcmp(inst->solver, "EXTRA_MIL") == 0) {
        extra_mileage(inst);
    } else if(strcmp(inst->solver, "GREEDY_ITER") == 0) {
        greedy_iterative(inst);
    } else if(strcmp(inst->solver, "GRASP_ITER") == 0) {
        grasp_iterative(inst);
    } else if(strcmp(inst->solver, "GREEDY_2OPT") == 0) {
        greedy_2opt(inst, 0);
    } else if(strcmp(inst->solver, "EXTRA_MIL_2OPT") == 0) {
        extra_mileage_2opt(inst);
    } else if(strcmp(inst->solver, "GRASP_2OPT") == 0) {
        grasp_2opt(inst, 0, 0.8);
    } else if(strcmp(inst->solver, "TABU_SEARCH") == 0) {
        //TODO chose initial solver
        greedy_iterative(inst);
        tabu_search(inst);
    } else if(strcmp(inst->solver, "VNS") == 0) {
        greedy_iterative(inst);
        vns(inst);
    } else if(strcmp(inst->solver, "CPLEX") == 0) {
        solve_cplex(inst);
    } else {
        print_error("Invalid solver selected");
    }
}

void solve_cplex(instance* inst) {
    TSPopt(inst);
}

void build_model(instance* inst, CPXENVptr env, CPXLPptr lp) {
    double zero = 0.0;
    char binary = 'B';

    char** cname = (char**) calloc(1, sizeof(char*));
    cname[0] = (char*) calloc(100, sizeof(char));

    //add binary variables
    for(int i=0; i<inst->nnodes; ++i) {
        for(int j=i+1; j<inst->nnodes; ++j) {
            sprintf(cname[0], "x(%d,%d)", i+1, j+1);
            double obj = get_cost(i, j, inst);
            double lb = 0.0;
            double ub = 1.0;

            int status = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname);
            if(status)
                print_error("Wrong CPXnewcols on x var.s");

            int cols = CPXgetnumcols(env, lp);
            if(cols-1 != xpos(i, j, inst))
                print_error("Wrong position for x var.s");
        }
    }

    //add degree constraints
    int* index = (int*) calloc(inst->nnodes, sizeof(int));
    double* value = (double*) calloc(inst->nnodes, sizeof(double));

    for(int i=0; i<inst->nnodes; ++i) {
        double rhs = 2.0;
        char sense = 'E';
        sprintf(cname[0], "degree(%d)", i+1);
        int nnz = 0;

        for(int j=0; j<inst->nnodes; ++j) {
            if(j == i) continue;
            index[nnz] = xpos(j, i, inst);
            value[nnz] = 1.0;
            nnz++;
        }

        int izero = 0;
        int status = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, &cname[0]);
        if(status)
            print_error("CPXaddrows(): error 1");
    }

    free(value);
    free(index);

    free(cname[0]);
    free(cname);

    //save lp file
    if(inst->verbose >= 100)
        CPXwriteprob(env, lp, "model.lp", NULL);
}

int TSPopt(instance* inst) {
    int error;
    //open CPLEX model
    CPXENVptr env = CPXopenCPLEX(&error);
    if(error)
        print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if(error)
        print_error("CPXcreateprob() error");

    build_model(inst, env, lp);

    //set CPLEX parameters
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
    CPXsetdblparam(env, CPX_PARAM_TILIM, inst->timelimit);
    //CPLEX output
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if(inst->verbose >= 10)
        CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);

    error = CPXmipopt(env, lp);
    if(error) {
        printf("CPX error code %d\n", error);
        print_error("CPXmipopt() error");
    }

    //use optimal solution found by CPLEX
    int ncols = CPXgetnumcols(env, lp);
    double* xstar = (double*) calloc(ncols, sizeof(double));
    
    //empty solution vector
    int* solution = (int*) calloc(inst->nnodes, sizeof(int));
    for(int i=0; i<inst->nnodes; ++i)
        solution[i] = -1;

    if(CPXgetx(env, lp, xstar, 0, ncols-1))
        print_error("CPXgetx() error");

    for(int i=0; i<inst->nnodes; ++i) {
        for(int j=i+1; j<inst->nnodes; ++j) {
            if(xstar[xpos(i, j, inst)] > 0.5) {
                printf("... x(%3d,%3d) = 1\n", i+1, j+1);

                /*TODO: add to solution vector
                if(solution[i] == -1) {
                    solution[i] = j;
                } else if(solution[i] != -1 && solution[j] == -1) {
                    solution[j] = i;
                } else {
                    solution[solution[j]] = j;
                    solution[j] = i;
                }*/
            }
        }
    }

    //check_solution(solution, inst->nnodes);
    memcpy(inst->best_sol, solution, inst->nnodes*sizeof(int));

    free(solution);
    free(xstar);

    //free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return 0;
}
