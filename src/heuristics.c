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

    //TODO use an edge struct, probably the best implementation
}

void grasp(instance* inst, int start_node) {

}
