#include "metaheuristics.h"
#include "heuristics.h"
#include <time.h>

void tabu_search(instance* inst) {
    //track execution time
    time_t start, end;
    time(&start);

    //best objective and solution
    double best_obj = inst->zbest;
    int* best_solution = calloc(inst->nnodes, sizeof(int));

    //current objective and solution
    double curr_obj = inst->zbest;
    int* curr_solution = calloc(inst->nnodes, sizeof(int));
    memcpy(curr_solution, inst->best_sol, inst->nnodes * sizeof(int));

    //tabu search variables
    int tnow = -1;
    int tenure = 20;    //TODO explore different approaches for the tenure value
    int* tabu_vector = calloc(inst->nnodes, sizeof(int));

    for(int i=0; i<inst->nnodes; ++i)
        tabu_vector[i] = -1000;

    int i = 0;
    
    while(1) {
        //update tnow
        tnow++;

        time(&end);
        if(difftime(end, start) > inst->timelimit)
            break;

        progressbar((int) difftime(end, start), inst->timelimit);

        //nodes involved
        int nodeA = -1;
        int nodeB = -1;
        int nodeC = -1;
        int nodeD = -1;

        //----------
        //OPTIMIZATION PART
        //----------

        curr_obj = alg_2opt(inst, curr_solution);

        if(curr_obj < best_obj) {
            //printf("BETTER SOLUTION FOUND WITH Z = %f\n", curr_obj);
            best_obj = curr_obj;
            memcpy(best_solution, curr_solution, inst->nnodes * sizeof(int));
        }

        //-----------------
        //RANDOM 2-OPT MOVE
        //-----------------

        //perform a 2-opt move for a new worse neighbor solution
        do {
            //consider edges A-C and B-D
            nodeA = rand() % inst->nnodes;
            nodeB = rand() % inst->nnodes;
            nodeC = curr_solution[nodeA];
            nodeD = curr_solution[nodeB];
            //printf("Selected edges [%d]-[%d] and [%d]-[%d]\n", nodeA, nodeC, nodeB, nodeD);
        } while(nodeA == nodeB || nodeB == nodeC || nodeA == nodeD);

        double old_cost = get_cost(nodeA, nodeC, inst) + get_cost(nodeB, nodeD, inst);

        //check if nodeA is in tabu vector
        if(tnow - tabu_vector[nodeA] <= tenure)
            continue;

        //reverse path from C to B;
        reverse_path(curr_solution, nodeC, nodeB);

        //swap edges A-C, B-D to A-B and C-D  
        curr_solution[nodeA] = nodeB;
        curr_solution[nodeC] = nodeD;

        //update current objective
        curr_obj += get_cost(nodeA, nodeB, inst) + get_cost(nodeC, nodeD, inst) - old_cost;

        //insert nodeA in tabu
        tabu_vector[nodeA] = tnow;

        check_solution(curr_solution, inst->nnodes);
    }

    check_solution(best_solution, inst->nnodes);

    memcpy(inst->best_sol, best_solution, inst->nnodes * sizeof(int));

    printf("FINAL BEST OBJECTIVE = [%f]\n", best_obj);

    free(best_solution);
    free(curr_solution);
    free(tabu_vector);
}

void vns(instance* inst) {
    //track execution time
    time_t start, end;
    time(&start);

    //best objective and solution
    double best_obj = inst->zbest;
    int* best_solution = calloc(inst->nnodes, sizeof(int));

    //current objective and solution
    double curr_obj = inst->zbest;
    int* curr_solution = calloc(inst->nnodes, sizeof(int));
    memcpy(curr_solution, inst->best_sol, inst->nnodes * sizeof(int));

    int i = 0;
    
    while(1) {
        time(&end);
        if(difftime(end, start) > inst->timelimit)
            break;

        progressbar((int) difftime(end, start), inst->timelimit);

        //Optimization part
        curr_obj = alg_2opt(inst, curr_solution);

        if(curr_obj < best_obj) {
            printf("BETTER SOLUTION FOUND WITH Z = %f\n", curr_obj);
            best_obj = curr_obj;
            memcpy(best_solution, curr_solution, inst->nnodes * sizeof(int));
        }

        //Double 3 opt move
        shake(inst, curr_solution);
        shake(inst, curr_solution);
    }

    check_solution(best_solution, inst->nnodes);

    memcpy(inst->best_sol, best_solution, inst->nnodes * sizeof(int));

    printf("FINAL BEST OBJECTIVE = [%f]\n", best_obj);

    free(best_solution);
    free(curr_solution);
}
