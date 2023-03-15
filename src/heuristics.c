#include "heuristics.h"

double greedy(instance* inst, int start_node) {
    //check start node validity
    if(start_node < 0 || start_node >= inst->nnodes)
        print_error("Invalid start node");

    //TODO track execution time
    
    //TODO compute all distances at the beginnig (should be the best approach)
    compute_distances(inst);

    //allocate memory for the solution (does not work here)
    //inst->best_sol = (int *) calloc(inst->nnodes, sizeof(int));

    //TODO use a local solution vector and then copy it into the best_sol instance attributes ?
    int* solution = (int* ) calloc(inst->nnodes, sizeof(int));

    //initialize all the cell to the corresponding index
    for(int i=0; i<inst->nnodes; ++i)
        solution[i] = i;
    // set start point at the beginning
    solution[0] = start_node;
    solution[start_node] = 0;

    //solution value
    double value = 0;

    // TODO use a visited array with more memory or search in the visited part of the solution array which is time consuming
    // visited array
    //int* visited = (int*) calloc(inst->nnodes, sizeof(int));
    //visited[start_node] = 1;

    //current node
    int curr_node = start_node;
    //current length of visited nodes in solution array
    int len = 1;

    //loop for the algorithm
    while(len < inst->nnodes-1) {
        //printf("ITERATION: [%d]\n", len);
        /*for(int i=0; i<inst->nnodes; ++i)
            printf("%d ", inst->best_sol[i]);
        printf("\n");*/
        int index = len;
        double min =__DBL_MAX__;

        //printf("NODE [%d]\n", curr_node);

        //search the next node
        for(int j=len; j<inst->nnodes; ++j) {
            int cand_node = solution[j];
            if(curr_node == cand_node)
                continue;
            double cost = get_cost(curr_node, cand_node, inst);
            //printf("Distance from node [%d] (%f, %f) = %f\n", inst->best_sol[j], inst->points[inst->best_sol[j]].x, inst->points[inst->best_sol[j]].y, cost);
            if(cost < min) {
                index = j;
                min = cost;
            }
        }

        //printf("Nearest node: %d, index: %d\n", inst->best_sol[index], index);

        //swap nodes
        int tmp = solution[len];
        solution[len] = solution[index];
        solution[index] = tmp;

        //update current node
        curr_node = solution[len];

        //update len
        len++;

        //increase solution value
        value += min;
    }

    //update objective value
    value += get_cost(start_node, solution[inst->nnodes], inst);

    //update model solution
    update_solution(value, solution, inst);

    //printf("Start node [%d] - Solution value: %f\n", start_node, value);
    
    return value;
}

void greedy_iterative(instance* inst) {
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
    //compute all the distances
    compute_distances(inst);

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

    //update solution format
    int prev = 0;
    for(int i=0; i<inst->nnodes; ++i) {
        inst->best_sol[i] = succ[prev];
        prev = succ[prev];
    }
    inst->zbest = z;

    free(succ);

    printf("Extra Mileage heuristic z = %f\n", z);
}

void grasp(instance* inst, int start_node) {

}
