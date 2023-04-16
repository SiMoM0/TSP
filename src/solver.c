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
    } else {
        print_error("Invalid solver selected");
    }
}

void solve_cplex(instance* inst) {

}
