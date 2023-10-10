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
 * @param inst model instance
*/
void vns(instance* inst);


/**
 * Genetic algorithm
 * 
 * @param inst model instance
*/
void genetic(instance* inst);