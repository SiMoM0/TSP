#include "metaheuristics.h"
#include "heuristics.h"

#define POPULATION 100
#define OFFSPRING_RATE 0.5 //must be 0.5 or less //tune:0.5,0.4,0.3,0.2,0.1
#define MUTATION_RATE 0.09 //0.03,0.06,0.09
#define MAX_GEN 1000 
#define INFBOUND                2147483647

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

    // step behavior (from min to max tenure)
    int min_tenure = 1;
    int max_tenure = (int) inst->nnodes * 0.1;

    // linear behavior (+1 to max tenure, then -1 to min tenure)
    int value = 1;

    for(int i=0; i<inst->nnodes; ++i)
        tabu_vector[i] = -1000;
    
    while(1) {
        //update tnow
        tnow++;

        // step behavior
        if(tnow % 2 == 0)
            tenure = min_tenure;
        else
            tenure = max_tenure;
        
        // linear behavior
        /*if(tenure >= max_tenure && value == 1)
            value = -1;
        else if(tenure <= min_tenure && value == -1)
            value = 1;
        tenure += value;*/

        // sin behavior (1 - max)
        //tenure = imax(1, (int) max_tenure * sin(tnow));

        // random behavior (1 - max)
        //tenure = imax(1, rand() % max_tenure);

        time(&end);
        double elapsed_time = difftime(end, start);
        if(difftime(end, start) > inst->timelimit) {
            inst->exec_time = elapsed_time;
            break;
        }

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

    time(&end);
    double elapsed_time = difftime(end, start);
    inst->exec_time = elapsed_time;

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
        double elapsed_time = difftime(end, start);
        if(difftime(end, start) > inst->timelimit) {
            inst->exec_time = elapsed_time;
            break;
        }

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
        shake(inst, curr_solution);
    }

    check_solution(best_solution, inst->nnodes);

    time(&end);
    double elapsed_time = difftime(end, start);
    inst->exec_time = elapsed_time;

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
        double elapsed_time = difftime(end, start);
        if(difftime(end, start) > inst->timelimit) {
            inst->exec_time = elapsed_time;
            break;
        }

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

        //printf("Curr obj = %6.2f | Delta = %4.2f | Norm Delta = %4.2f | Threshold = %4.4f | Prob = %2.4f\n", curr_obj, delta, delta_z, threshold, probability);

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

    time(&end);
    double elapsed_time = difftime(end, start);
    inst->exec_time = elapsed_time;

    update_solution(curr_obj, curr_solution, inst);

    free(curr_solution);
}


// member of the population
typedef struct {
	int* solution;      // tsp feasible tour
	double fitness;     // score of the member -> it is a penalized cost
	double wheelProb;   // fitness / totalFitness
	double sumProb;     // cumulative probability
} chromosome;

void computeFitness(instance* inst, int* solution, int* visited, double* cost) {
	int index = 0;
	int next = -1;
	do {
		next = solution[index];
		*cost += get_cost(index, next, inst);
		index = next;

	} while (index != 0);
}

int compareChromosomes(const void* lhs, const void* rhs) {
	const chromosome* lp = lhs;
	const chromosome* rp = rhs;

	return lp->fitness - rp->fitness;
}

//uses the wheel roulette method to select a parent
int selectParent(chromosome* population) {

	// Select a random number between 0 and 1
	double random = (double)rand() / RAND_MAX;

	// Use the roulette wheel to select a chromosome
	for (int i = POPULATION - 1; i >= 0; i--) {
		if (random <= population[i].sumProb) return i;
	}

	print_error("ROULETTE WHEEL FAILED");
	exit(1);
}

void computeExtra(instance* inst, int* visited, int* solution, double* cost) {

	//take generic node, non visited
	for (int z = 0; z < inst->nnodes; z++) {
		if (visited[z]) continue;    //skip visited nodes

		double min = INFBOUND;
		int imin = -1;
		int jmin = -1;

		//for every visited  node i
		for (int i = 0; i < inst->nnodes; i++) {
			if (!visited[i]) continue;   //skip non visited nodes

			double extraM = get_cost(i, z, inst) + get_cost(z, solution[i], inst) - get_cost(i, solution[i], inst);

			if (extraM < min) {
				min = extraM;
				imin = i;
			}

		}

		jmin = solution[imin];
		solution[imin] = z;
		solution[z] = jmin;
		visited[z] = 1;

		*cost += min;
	}
}


//produce the crossover given two parents
void produceOffspring(instance* inst, int* p1, int* p2, chromosome* offspring) {

	int breakPoint = rand() % (inst->nnodes - 2) + 1;

	int* visited = (int*)calloc(inst->nnodes, sizeof(int));

	int index = 0;
	visited[0] = 1;
	int temp = -1;

	//copy first half of p1 into the child
	for (int i = 0; i < breakPoint; i++) {
		temp = p1[index];
		offspring->solution[index] = temp;
		visited[temp] = 1;
		index = temp;
	}

	//copy second half of p2 into the child
	for (int i = breakPoint; i < inst->nnodes; i++) {
		temp = p2[temp];
		if (!visited[temp]) {
			offspring->solution[index] = temp;
			visited[temp] = 1;
			index = temp;
		}
	}

	offspring->solution[index] = 0;

	offspring->fitness = 0;

	//updates fitness of the newborn
	computeFitness(inst, offspring->solution, visited, &offspring->fitness);

	//repair solution using extra mileage //DA VEDERE MEGLIO
	computeExtra(inst, visited, offspring->solution, &offspring->fitness);

	free(visited);
}


//initializes the starting population
int initMember(instance* inst, chromosome* member) {

	member->solution = (int*)calloc(inst->nnodes, sizeof(int));

	int start_node = 0 + ((int) ( ((double) random()) / RAND_MAX * (inst->nnodes - 0)));
    //grasp_iterative(inst);
	grasp(inst, start_node, 0.5);

	//initialize each member with a random sol. from grasp
	memcpy(member->solution, inst->best_sol, sizeof(int) * inst->nnodes);
	member->fitness = inst->zbest;

	inst->zbest = -1;

	if (inst->verbose >= 10) {
		//if (checkSol(inst, member->solution)) return 1;
        check_solution(member->solution, inst->nnodes);
		check_cost(inst, member->solution, member->fitness);
	}

	return 0;
}

//resets offspring and visited for the new generation
void reset(instance* inst, chromosome* offspring, int* visited) {

	for (int i = 0; i < POPULATION * OFFSPRING_RATE; i++) {
		memset(offspring[i].solution, 0, sizeof(int) * inst->nnodes);
		offspring[i].fitness = 0;
	}

	memset(visited, 0, sizeof(int) * POPULATION);
}


void genetic(instance* inst) {

    //inst->timeStart = second();
    time_t start, end;
    time(&start);

	chromosome* population = (chromosome*)calloc(POPULATION, sizeof(chromosome));
	double totalFit = 0;

	// initialize population
	for (int i = 0; i < POPULATION; i++) {
		if (initMember(inst, &population[i])) printf("Error init_population");
		totalFit += population[i].fitness;
	}

	if (inst->verbose >= 1) printf("\n-- POPULATION INITIALIZED SUCCESS --\n");
	

	int gen = 1;

	//initializes offspring
	chromosome* offspring = (chromosome*)calloc(POPULATION * OFFSPRING_RATE, sizeof(chromosome));
	for (int i = 0; i < POPULATION * OFFSPRING_RATE; i++) {
		offspring[i].solution = (int*)calloc(inst->nnodes, sizeof(int));
	}

	//initializes the 0-1 array to select the parents
	int* visited = (int*)calloc(POPULATION, sizeof(int));

	//initializes the mutant
	chromosome mutant;
	mutant.solution = (int*)calloc(inst->nnodes, sizeof(int));


	do {

        time(&end);
        double elapsed_time = difftime(end, start);
        if(difftime(end, start) > inst->timelimit) {
            inst->exec_time = elapsed_time;
            break;
        }

        progressbar((int) difftime(end, start), inst->timelimit);

		//compute the wheel roulette probabilities
		for (int i = 0; i < POPULATION; i++) {
			population[i].wheelProb = population[i].fitness / totalFit;
		}

		//sort population based on fitness. First we have the one with better fitness, last the worse
		qsort(population, POPULATION, sizeof(chromosome), compareChromosomes);

		//check to update the best solution
		if (inst->zbest == -1 || inst->zbest > population[0].fitness) {
            update_solution(population[0].fitness, population[0].solution, inst);
			//updateSol(inst, population[0].fitness, population[0].solution);
		}

		//compute the cumulative probabilities to use in the wheel roulette
		population[0].sumProb = 1;
		for (int i = 1; i < POPULATION; i++) {
			population[i].sumProb = population[i - 1].sumProb - population[i - 1].wheelProb;
		}


		int count = 0;

		//produce the entire offpring
		while (count < POPULATION * OFFSPRING_RATE) {

			int p1, p2; //the parents

			do { p1 = selectParent(population); } while (visited[p1]);
			visited[p1] = 1;

			do { p2 = selectParent(population); } while (p1 == p2 || visited[p2]);
			visited[p2] = 1;

			//crossover
			produceOffspring(inst, population[p1].solution, population[p2].solution, &offspring[count]);

			if (inst->verbose >= 10) {
				check_solution(offspring[count].solution, inst->nnodes);
				check_cost(inst, offspring[count].solution, offspring[count].fitness);
			}

			count++;
		}

		//apply a mutation by using 2opt
		double m = (double)rand() / RAND_MAX;
		int mutantIndex = -1;

		if (m < MUTATION_RATE) {
			mutantIndex = rand() % (POPULATION - 1) + 1;   //assures that the first element remains elite

			memcpy(mutant.solution, population[mutantIndex].solution, sizeof(int) * inst->nnodes);
			mutant.fitness = population[mutantIndex].fitness;

			//use 2opt to create a mutation
            mutant.fitness = alg_2opt(inst, mutant.solution);
    
			//printf("new fitness: %f", mutant.fitness);

			if (inst->verbose >= 10) {
				check_solution(mutant.solution, inst->nnodes);
				check_cost(inst, mutant.solution, mutant.fitness);
			}

		}

		//create the new generation keeping the elitism
		int offset = ((POPULATION - POPULATION * OFFSPRING_RATE));

		//copy the offspring at the end of the population, killing all worse chromosomes
		for (int i = offset; i < POPULATION; i++) {
			memcpy(population[i].solution, offspring[i - offset].solution, sizeof(int) * inst->nnodes);
			population[i].fitness = offspring[i - offset].fitness;
		}


		//add mutant to the survivors (if there is). It adds it right before the newly added offspring
		if (mutantIndex != -1) {
			memcpy(population[(int)(POPULATION * OFFSPRING_RATE) - 1].solution, mutant.solution, sizeof(int) * inst->nnodes);
			population[(int)(POPULATION * OFFSPRING_RATE) - 1].fitness = mutant.fitness;
		}

		for (int i = 0; i < POPULATION; i++) {
			check_cost(inst, population[i].solution, population[i].fitness);
		}

		gen++;

		totalFit = 0;

		//if there is a new generation to start, resets the useful arrays and updates the total fitness
		if (gen <= MAX_GEN) {
			reset(inst, offspring, visited);
			for (int i = 0; i < POPULATION; i++) {
				totalFit += population[i].fitness;
			}
		}


	} while (gen <= MAX_GEN);

	//checks last generation for the best solution
	//sort population based on fitness. First we have the one with better fitness, last the worse
	qsort(population, POPULATION, sizeof(chromosome), compareChromosomes);

	if (inst->zbest == -1 || inst->zbest > population[0].fitness) {
        update_solution(population[0].fitness, population[0].solution, inst);
	}

    time(&end);
    double elapsed_time = difftime(end, start);
    inst->exec_time = elapsed_time;

    //debug_plot(inst, inst->best_sol);
	if (inst->verbose >= 1)printf("BEST GENETIC COST FOUND: %f\n", inst->zbest);
	if (inst->verbose >= 10)printf("generation: %d\n", gen);

	free(visited);

	free(mutant.solution);

    for (int i = 0; i < POPULATION * OFFSPRING_RATE; i++) {
		free(offspring[i].solution);
	}
	free(offspring);

    for (int i = 0; i < POPULATION; i++) {
		free(population[i].solution);
	}
	free(population);
}
