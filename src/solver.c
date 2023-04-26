#include "solver.h"

void solve(instance* inst){
    if (inst->cplex == 1) {     // Solve using cplex
        TSPopt(inst);
    } else {                                // Solve using our heuristic methods
        solve_heuristic(inst);
    }
     
}

int solve_problem(CPXENVptr env, CPXLPptr lp, instance *inst) {
    int status;
    if(strncmp(inst->solver, "BENDERS", 7) == 0) {
        status = Benders(inst, env, lp);
        
    }
    return status;
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
        //TODO choose initial solver
        greedy_iterative(inst);
        tabu_search(inst);
    } else if(strcmp(inst->solver, "VNS") == 0) {
        greedy_iterative(inst);
        vns(inst);
    } else {
        print_error("Invalid solver selected");
    }
}



