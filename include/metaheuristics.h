//METAHEURISTIC ALGORITHMS

#pragma once

#include "utils.h"

/**
 * Tabu Search algorithm
 * 
 * @param inst model instance
*/
void tabu_search(instance* inst);

/**
 * VNS algorithm
 * 
 * @param inst model isntance
*/
void vns(instance* inst);

/**
 * Simulated Annealing algorithm
 * 
 * @param isnt model instance
*/
void simulated_annealing(instance* inst);
