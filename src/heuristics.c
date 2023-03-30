#include "heuristics.h"

double greedy(instance* inst, int start_node) {
    //check start node validity
    if(start_node < 0 || start_node >= inst->nnodes)
        print_error("Invalid start node");

    //TODO track execution time

    int* solution = (int* ) calloc(inst->nnodes, sizeof(int));

    //initialize all the cell to -1
    for(int i=0; i<inst->nnodes; ++i)
        solution[i] = -1;

    //solution objective value z
    double z = 0;

    //current node
    int curr_node = start_node;
    //current length of visited nodes
    int len = 0;

    //loop for the algorithm
    while(len < inst->nnodes-1) {
        //successor node
        int succ_node = start_node;
        double min_cost =__DBL_MAX__;

        //search the next node
        for(int i=0; i<inst->nnodes; ++i) {
            //check nodes not visited yet
            if(curr_node == i || solution[i] != -1)
                continue;
            
            double cost = get_cost(curr_node, i, inst);
            if(cost < min_cost) {
                succ_node = i;
                min_cost = cost;
            }
        }

        //set successor node
        solution[curr_node] = succ_node;

        //update current node
        curr_node = succ_node;

        //update len
        len++;

        //increase solution value
        z += min_cost;
    }

    //set successor of last node visited
    solution[curr_node] = start_node;

    //check solution validity
    check_solution(solution, inst->nnodes);

    //update objective value
    z += get_cost(start_node, curr_node, inst);

    //update model solution
    update_solution(z, solution, inst);

    free(solution);

    printf("Start node [%d] - Solution value: %f\n", start_node, z);
    
    return z;
}

void greedy_iterative(instance* inst) {
    //TODO track execution time
    int best_node = 0;
    double best_cost = __DBL_MAX__;

    for(int i=0; i<inst->nnodes; ++i) {
        double cost = greedy(inst, i);
        if(cost < best_cost) {
            best_node = i;
            best_cost = cost;
        }
    }

    printf("Greedy heuristic - Best starting node = [%d] - Cost = [%f]\n", best_node, best_cost);
}

void extra_mileage(instance* inst) {
    //TODO track execution time

    //furthest nodes
    int node1 = 0;
    int node2 = 1;

    double max_dist = 0;

    //find furthest nodes
    for(int i=0; i<inst->nnodes; ++i) {
        for(int j=i+1; j<inst->nnodes; ++j) {
            double dist = get_cost(i, j, inst);
            if(dist > max_dist) {
                node1 = i;
                node2 = j;
                max_dist = dist;
            }
        }
    }

    //using succ vector where element j in cell i means node j is successor of node i
    int* succ = calloc(inst->nnodes, sizeof(int));

    //initialize all successors to -1
    for(int i=0; i<inst->nnodes; ++i) {
        succ[i] = -1;
    }

    //set two nodes at maximum distance
    succ[node1] = node2;
    succ[node2] = node1;

    //number of edges in the tour
    int tour_len = 2;

    //solution objective z
    double z = 2 * get_cost(node1, node2, inst);

    while(tour_len < inst->nnodes) {
        //candinate node to enter the tour
        int enter_node = 0;
        double enter_cost = __DBL_MAX__;

        //scan through all edges
        for(int i=0; i<inst->nnodes; ++i) {
            //check if edge exists
            if(succ[i] == -1)
                continue;

            //current edge info
            int nodeA = i;
            int nodeB = succ[i];

            //find new node to enter the tour
            for(int j=0; j<inst->nnodes; ++j) {
                //exclude already visited nodes
                if(succ[j] != -1)
                    continue;

                //compute cost difference
                double cost_difference = get_cost(nodeA, j, inst) + get_cost(j, nodeB, inst) - get_cost(nodeA, nodeB, inst);

                //in case update edge info
                if(cost_difference < enter_cost) {
                    enter_node = j;
                    node1 = nodeA;
                    node2 = nodeB;
                    enter_cost = cost_difference;
                }
            }
        }

        //update succ vector
        succ[enter_node] = succ[node1];
        succ[node1] = enter_node;

        //update solution objective
        z += enter_cost;

        //increment tour length
        tour_len++;
    }

    //check solution
    check_solution(succ, inst->nnodes);

    update_solution(z, succ, inst);

    free(succ);

    printf("Extra Mileage heuristic z = %f\n", z);
}

double grasp(instance* inst, int start_node, double p) {
    //check start node validity
    if(start_node < 0 || start_node >= inst->nnodes)
        print_error("Invalid start node");

    //TODO track execution time

    //set probability
    double p1 = p;
    double p2 = 1 - p1;

    int* solution = (int* ) calloc(inst->nnodes, sizeof(int));

    //initialize all the cell to -1
    for(int i=0; i<inst->nnodes; ++i)
        solution[i] = -1;

    //solution objective value z
    double z = 0;

    //current node
    int curr_node = start_node;
    //current length of visited nodes
    int len = 0;

    //loop for the algorithm
    while(len < inst->nnodes-1) {
        //successor nodes
        int succ_first = -1;
        double first_cost =__DBL_MAX__;
        int succ_second = -1;
        double second_cost = __DBL_MAX__;

        //search the next node
        for(int i=0; i<inst->nnodes; ++i) {
            //check nodes not visited yet
            if(curr_node == i || solution[i] != -1)
                continue;
            
            double cost = get_cost(curr_node, i, inst);
            if(cost < first_cost) {
                succ_second = succ_first;
                second_cost = first_cost;
                succ_first = i;
                first_cost = cost;
            }
        }

        //get random probability
        double prob = (double) rand() / RAND_MAX;

        //set successor node, update current node and objective
        if(prob > p2 || succ_second == -1) {
            solution[curr_node] = succ_first;
            curr_node = succ_first;
            z += first_cost;
        }
        else {
            solution[curr_node] = succ_second;
            curr_node = succ_second;
            z += second_cost;
        }

        //update len
        len++;
    }

    //set successor of last node visited
    solution[curr_node] = start_node;

    //check solution validity
    check_solution(solution, inst->nnodes);

    //update objective value
    z += get_cost(start_node, curr_node, inst);

    //update model solution
    update_solution(z, solution, inst);

    free(solution);

    //printf("Start node [%d] with p1 [%f] - Solution value: %f\n", start_node, p, z);
    
    return z;
}

void grasp_iterative(instance* inst) {
    //TODO track execution time
    int best_node = 0;
    double best_cost = __DBL_MAX__;
    double p = 0.8;

    for(int i=0; i<inst->nnodes; ++i) {
        //p = (double) rand() / RAND_MAX;
        double cost = grasp(inst, i, p);
        if(cost < best_cost) {
            best_node = i;
            best_cost = cost;
        }
    }

    printf("Grasp heuristic - Best starting node = [%d] with p1 = [%f] - Cost = [%f]\n", best_node, p, best_cost);
}

double alg_2opt(instance* inst, int* input_solution) {
    double z = 0;
    for(int i=0; i<inst->nnodes; ++i)
        z += get_cost(i, input_solution[i], inst);

    int improve = 1;
    while(improve) {
        improve = 0;
        
        //Consider current crossing edges A-D and C-B
        //save nodes involved in the edges
        int nodeA = -1;
        int nodeB = -1;
        int nodeC = -1;
        int nodeD = -1;

        double best_delta = 0;

        //loop through all edges
        for(int i=0; i<inst->nnodes-1; ++i) {
            for(int j=i+1; j<inst->nnodes; ++j) {
                //skip consecutive edges
                if(input_solution[i] == j || input_solution[j] == i)
                    continue;

                //current edges weight of A-D and C-B
                double curr_weight = get_cost(i, input_solution[i], inst) + get_cost(j, input_solution[j], inst);
                //weight considering new edges C-A and B-D
                double new_weight = get_cost(j, i, inst) + get_cost(input_solution[j], input_solution[i], inst);
                double delta = curr_weight - new_weight;

                //save new best edges to swap
                if(delta > best_delta) {
                    //printf("Previous edges (%d-%d) (%d-%d)\n", i, input_solution[i], j, input_solution[j]);
                    //printf("New better edge found between (%d, %d) and (%d, %d) with delta = [%f]\n", j, i, input_solution[j], input_solution[i], delta);
                    improve = 1;
                    nodeA = i;
                    nodeC = j;
                    nodeD = input_solution[i];
                    nodeB = input_solution[j];
                    best_delta = delta;
                }
            }
        }
        if(!improve)
            break;

        reverse_path(input_solution, nodeB, nodeA);

        //set new edges C-A B-D
        input_solution[nodeB] = nodeD;
        input_solution[nodeC] = nodeA;

        check_solution(input_solution, inst->nnodes);

        //update objective
        z -= best_delta;
    }

    //printf("COMPLETED 2-OPT with z = [%f]\n\n", z);

    return z;
}

void greedy_2opt(instance* inst, int start_node) {
    greedy(inst, start_node);

    int* curr_solution = calloc(inst->nnodes, sizeof(int));
    memcpy(curr_solution, inst->best_sol, inst->nnodes * sizeof(int));

    double z = alg_2opt(inst, curr_solution);
    update_solution(z, curr_solution, inst);
    printf("New z after 2-opt = %f\n", z);

    free(curr_solution);
}

void extra_mileage_2opt(instance* inst) {
    extra_mileage(inst);

    int* curr_solution = calloc(inst->nnodes, sizeof(int));
    memcpy(curr_solution, inst->best_sol, inst->nnodes * sizeof(int));

    double z = alg_2opt(inst, curr_solution);
    update_solution(z, curr_solution, inst);
    printf("New z after 2-opt = %f\n", z);

    free(curr_solution);
}

void grasp_2opt(instance* inst, int start_node, double p) {
    grasp(inst, start_node, p);

    int* curr_solution = calloc(inst->nnodes, sizeof(int));
    memcpy(curr_solution, inst->best_sol, inst->nnodes * sizeof(int));

    double z = alg_2opt(inst, curr_solution);
    update_solution(z, curr_solution, inst);
    printf("New z after 2-opt = %f\n", z);

    free(curr_solution);
}
