// HEURISTIC ALGORITHMS

#pragma once

#include "utils.h"

/**
 * Greedy heuristic (Nearest Neighbor)
 * 
 * @param inst model instance
 * @param start_node index of the starting node
 * @return cost of the solution
*/
double greedy(instance* inst, int start_node);

/**
 * Greedy heuristic performed on every node in order to find the best starting node
 * 
 * @param inst model isntance
*/
void greedy_iterative(instance* inst);

/**
 * Extra Mileage heuristic
 * 
 * @param inst model instance
*/
void extra_mileage(instance* inst);

/**
 * Grasp heuristic (Nearest Neighbor with random choice)
 * 
 * @param inst model instance
 * @param start_node index of the starting node
 * @param p probability of selecting first best node
 * @return objective value
*/
double grasp(instance* inst, int start_node, double p);

/**
 * Grasp heuristic performed on every node
 * 
 * @param inst model instance
*/
void grasp_iterative(instance* inst);

/**
 * 2-opt algorithm
 * 
 * @param inst model instance
 * @param solution input solution
 * @return new cost of the solution
*/
double alg_2opt(instance* inst, int* input_solution);

/**
 * Greedy and 2-opt algorithm
 * 
 * @param inst model instance
 * @param start_node index of starting node
*/
void greedy_2opt(instance* inst, int start_node);

/**
 * Extra mileage algorithm and 2-opt
 * 
 * @param inst model instance
*/
void extra_mileage_2opt(instance* inst);

/**
 * Grasp and 2opt algorithm
 * 
 * @param inst model instance
 * @param start_node index of the starting node
 * @param p probability of selecting first best node
*/
void grasp_2opt(instance* inst, int start_node, double p);
