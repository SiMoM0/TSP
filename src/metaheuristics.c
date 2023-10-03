#include "metaheuristics.h"
#include "heuristics.h"

void tabu_search(instance* inst) {
    if(inst->zbest == -1)
        print_error("Initial solution is empty");

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
    int tenure = 10;    //TODO explore different approaches for the tenure value
    int* tabu_vector = calloc(inst->nnodes, sizeof(int));

    for(int i=0; i<inst->nnodes; ++i)
        tabu_vector[i] = -1000;
    
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

        //perform 2-opt considering tabu nodes
        int improve = 1;
        while(improve) {
            improve = 0;

            double best_delta = 0;

            //loop through all edges
            for(int i=0; i<inst->nnodes-1; ++i) {
                //skip tabu node
                if(tnow - tabu_vector[i] <= tenure || tnow-tabu_vector[curr_solution[i]] <= tenure)
                    continue;

                for(int j=i+1; j<inst->nnodes; ++j) {
                    //skip consecutive edges
                    if(curr_solution[i] == j || curr_solution[j] == i)
                        continue;

                    //skip tabu nodes
                    if(tnow - tabu_vector[j] <= tenure || tnow - tabu_vector[curr_solution[j]] <= tenure)
                        continue;

                    //current edges weight of A-D and C-B
                    double curr_weight = get_cost(i, curr_solution[i], inst) + get_cost(j, curr_solution[j], inst);
                    //weight considering new edges C-A and B-D
                    double new_weight = get_cost(j, i, inst) + get_cost(curr_solution[j], curr_solution[i], inst);
                    double delta = curr_weight - new_weight;

                    //save new best edges to swap
                    if(delta > best_delta) {
                        //printf("New better edge found between (%d, %d) and (%d, %d) with delta = [%f]\n", j, i, curr_solution[j], curr_solution[i], delta);
                        improve = 1;
                        nodeA = i;
                        nodeC = j;
                        nodeD = curr_solution[i];
                        nodeB = curr_solution[j];
                        best_delta = delta;
                    }
                }
            }
            if(!improve)
                break;

            reverse_path(curr_solution, nodeB, nodeA);

            //set new edges C-A  and B-D
            curr_solution[nodeB] = nodeD;
            curr_solution[nodeC] = nodeA;

            check_solution(curr_solution, inst->nnodes);

            //update objective
            curr_obj -= best_delta;

            //update best solution
            if(curr_obj < best_obj) {
                printf("ITER [%d] - BETTER SOLUTION FOUND WITH Z = %f\n", tnow, curr_obj);
                best_obj = curr_obj;
                memcpy(best_solution, curr_solution, inst->nnodes * sizeof(int));
            }
        }
        //printf("BETTER CURRENT SOLUTION FOUND WITH Z = %f\n", curr_obj);
        //debug_plot(inst, curr_solution);

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
        //printf("CURRENT OBJECTIVE = [%f]\n", curr_obj);
        //debug_plot(inst, curr_solution);

        //insert nodeA in tabu
        tabu_vector[nodeA] = tnow;

        check_solution(curr_solution, inst->nnodes);
    }

    check_solution(best_solution, inst->nnodes);

    update_solution(best_obj, best_solution, inst);

    printf("FINAL BEST OBJECTIVE = [%f]\n", best_obj);

    free(best_solution);
    free(curr_solution);
    free(tabu_vector);
}

void vns(instance* inst) {
    if(inst->zbest == -1)
        print_error("Initial solution is empty");
        
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

    update_solution(best_obj, best_solution, inst);

    printf("FINAL BEST OBJECTIVE = [%f]\n", best_obj);

    free(best_solution);
    free(curr_solution);
}

void simulated_annealing(instance* inst) {
    if(inst->zbest == -1)
        print_error("Initial solution is empty");

    //track execution time
    time_t start, end;
    time(&start);

    //current objective and solution
    double curr_obj = inst->zbest;
    int* curr_solution = calloc(inst->nnodes, sizeof(int));
    memcpy(curr_solution, inst->best_sol, inst->nnodes * sizeof(int));

    // simulated annealing variables
    double alpha = 0.99;
    double T = (double) inst->nnodes * curr_obj; // seems the best
    //double T = (double) curr_obj * 10;

    while(1) {
        time(&end);
        if(difftime(end, start) > inst->timelimit)
            break;

        progressbar((int) difftime(end, start), inst->timelimit);

        //Consider random crossing edges A-D and C-B
        int nodeA = rand() % inst->nnodes;
        int nodeC = rand() % (inst->nnodes - 1);
        int nodeB = curr_solution[nodeC];
        int nodeD = curr_solution[nodeA];

        while(nodeA == nodeC || nodeA == nodeB || nodeC == nodeD) {
            nodeA = rand() % inst->nnodes;
            nodeC = rand() % (inst->nnodes - 1);
            nodeB = curr_solution[nodeC];
            nodeD = curr_solution[nodeA];
        }

        //current edges weight of A-D and C-B
        double curr_weight = get_cost(nodeA, nodeD, inst) + get_cost(nodeC, nodeB, inst);
        //weight considering new edges C-A and B-D
        double new_weight = get_cost(nodeC, nodeA, inst) + get_cost(nodeB, nodeD, inst);

        // delta between the two solutions normalized
        double delta = new_weight - curr_weight;
        double delta_z = delta / (curr_obj / inst->nnodes);

        // threshold
        // TODO stop when threshold is almost zero ?
        double threshold = exp(- delta_z / T);

        // probability
        double probability = (double) rand() / RAND_MAX;

        printf("Curr obj = %6.2f | Delta = %4.2f | Norm Delta = %4.2f | Threshold = %4.4f | Prob = %2.4f\n", curr_obj, delta, delta_z, threshold, probability);

        // in case accept the new solution
        if(probability < threshold) {
            reverse_path(curr_solution, nodeB, nodeA);

            //set new edges C-A B-D
            curr_solution[nodeB] = nodeD;
            curr_solution[nodeC] = nodeA;

            check_solution(curr_solution, inst->nnodes);

            curr_obj += delta;

            printf("New solution = %f\n", curr_obj);
        }

        // update T
        T *= alpha;
    }

    check_solution(curr_solution, inst->nnodes);

    update_solution(curr_obj, curr_solution, inst);

    free(curr_solution);
}
