#include "metaheuristics.h"
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
                    if(new_weight < curr_weight && delta > best_delta) {
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

            //reverse path from node B to node A
            int prev = nodeB;
            int node = curr_solution[nodeB];
            while(node != nodeD) {
                int next_node = curr_solution[node];
                curr_solution[node] = prev;
                prev = node;
                //update node
                node = next_node;
            }

            //set new edges C-A  and B-D
            curr_solution[nodeB] = nodeD;
            curr_solution[nodeC] = nodeA;

            check_solution(curr_solution, inst->nnodes);

            //update objective
            curr_obj -= best_delta;

            //update best solution
            if(curr_obj < best_obj) {
                best_obj = curr_obj;
                memcpy(best_solution, curr_solution, inst->nnodes * sizeof(int));
            }
        }

        //-----------------
        //RADNOM 2-OPT MOVE
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
