#include "vns.h"
#include "heuristics.h"


void shake(instance* inst) {
    //get instance solution and objective value
    int* solution = calloc(inst->nnodes, sizeof(int));
    memcpy(solution, inst->best_sol, inst->nnodes * sizeof(int));
    double z = inst->zbest;


    //print succ vector
    /*for(int i=0; i<inst->nnodes; i++){
        printf("\nsolution[%d] = %d", i, solution[i]);
    }
    */

    int idx1 =  rand() % inst->nnodes;
    int idx2=idx1;
    int idx3=idx1;
    while(idx2==idx1 || abs(idx1-idx2)<=1){    // no same node and not successor or predecessor idx1
        idx2=rand() % inst->nnodes;
        
    }
    while(idx3==idx1 || idx3==idx2 || abs(idx1-idx3)<=1 || abs(idx2-idx3)<=1){
        idx3=rand() % inst->nnodes;
    }
    //put them in order
    if(idx1>idx2){
        swap(&idx1, &idx2);
    }
    if(idx1>idx3){
        swap(&idx1, &idx3);
    }   
    if(idx2>idx3){
        swap(&idx2, &idx3);
    }
    //printf("\nidx1 = %d, idx2 = %d, idx3 = %d", idx1, idx2, idx3);
    
    //choose the nodes that are involved in the swap edges procedure
    int nodeA = idx1;
    int nodeB = solution[idx1];
    int nodeC = idx2;
    int nodeD = solution[idx2];
    int nodeE = idx3;
    int nodeF = solution[idx3];

    /*printf("\nNodeA = %d", nodeA);
    printf("\nNodeB = %d", nodeB);
    printf("\nNodeC = %d", nodeC);
    printf("\nNodeD = %d", nodeD);
    printf("\nNodeE = %d", nodeE);
    printf("\nNodeF = %d\n\n", nodeF);
    */

    //case 7
    solution[nodeA] = nodeD; //succ of A is D
    solution[nodeE] = nodeB; //succ of E is B
    solution[nodeC] = nodeF; //succ of C is F

    /*printf("\nNodeA = %d", nodeA);
    printf("\nsuccA = %d", solution[nodeA]);
    printf("\nNodeB = %d", nodeB);
    printf("\nsuccB = %d", solution[nodeB]);
    printf("\nNodeC = %d", nodeC);
    printf("\nsuccC = %d", solution[nodeC]);
    printf("\nNodeD = %d", nodeD);
    printf("\nsuccD = %d", solution[nodeD]);
    printf("\nNodeE = %d", nodeE);
    printf("\nsuccE = %d", solution[nodeE]);
    printf("\nNodeF = %d", nodeF);
    printf("\nsuccF = %d\n\n", solution[nodeF]);
    */
    
    check_solution(solution, inst->nnodes);
    
    compute_distances(inst);

    
    //compute the new cost 
    for(int i=0; i<inst->nnodes; i++) {
        z += get_cost(i,solution[i], inst);
    }
    //printf("\nzbest after edges swap = %f", z);
    

    //NOTA: non fa l'update se z > zbest!!!
    update_solution(z, solution, inst);
    //i am setting manually z_best and best_sol
    //inst->zbest = z;
	//memcpy(inst->best_sol, solution, inst->nnodes * sizeof(int));
    free(solution);
}

int VNS(instance* inst) {
    int updated = 0;
    //setting the time limit
    time_t start, end;
    time(&start);

    //compute the 2-opt alg of current instance
    int improve1 = alg_2opt(inst);
    //printf("\nimprove1 = %d\n", improve1);

    //store locally the main info: best_solution, zbest 
    int* solution = calloc(inst->nnodes, sizeof(int));
    memcpy(solution, inst->best_sol, inst->nnodes * sizeof(int));
    double z = inst->zbest;

    while(1) {

        time(&end);
        if(difftime(end, start) > inst->timelimit)
            break;
    
        //Modify current solution to a random point in the neighboorhood
        //NOTA: va a modificare la current instance (non dovrebbe forse)
        //apply two times the shake fuction in order to modify 6 edges in total
        shake(inst);
        shake(inst);

        //printf("\n\nz_after shake: %f\n\n", inst->zbest);

        //compute the 2-opt of the new solution
        alg_2opt(inst); 
        printf("\nz_best after 2-opt of swapped edges: %f", inst->zbest);
        //If I found that this solution is better than the best, update the best solution with the current one
        if (inst->zbest < z) {
            z = inst->zbest;
            memcpy(solution, inst->best_sol, inst->nnodes * sizeof(int));         
            printf("\nsto aggiornando\nAdesso in z (che è il current_best) c'è %f", z);   
        }
        //TO DO non so se va spostato dentro l'if oppure no, perchè così sto aggiornando a prescindere
        //però se non aggiorno parto sempre dalla stessa solutione per poi darla in input alla funzione shake
        //NOTA: se la soluzione trovata è migliore della best, l'update solution è inutile
        //perchè in inst ci sono già i valori ottimali
        //NOTA: se, invece, la soluzione trovata è peggiore della best, 
        //faccio l'update con i dati della best
        //update_solution(z, solution, inst);

    }
    
    update_solution(z, solution, inst);

    free(solution);

    return updated;
}
