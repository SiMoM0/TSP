// UTILS FILE

#pragma once

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "tsp.h"

/**
 * Calculate the maximum between two integers
 * 
 * @param i1 first integer value
 * @param i2 second integer value
 * @return the maximum integer
*/
int imax(int i1, int i2);

/**
 * Calculate the minimum between two integers
 * 
 * @param i1 first integer value
 * @param i2 second integer value
 * @return the minimum integer
*/
int imin(int i1, int i2);

/**
 * Calculate the maximum between two doubles
 * 
 * @param i1 first double value
 * @param i2 second double value
 * @return the maximum double
*/
double dmax(double d1, double d2);

/**
 * Calculate the minimum between two doubles
 * 
 * @param i1 first double value
 * @param i2 second double value
 * @return the minimum double
*/
double dmin(double d1, double d2);

/**
 * Print error message
 * 
 * @param err string of the error
*/
void print_error(const char *err);

/**
 * Print debug message
 * 
 * @param err string of the debug message
*/
void debug(const char *err);

/**
 * Create a path for a file with build directory as reference
 * 
 * @param output_path output path created
 * @param dir directory name
 * @param file file name
 * @param ext file extension
*/
void create_path(char* output_path, char* dir, char* filename, char* ext);

/**
 * Read inputs and create tsp instance
 * 
 * @param inst model instance
*/
void parse_model(instance *inst);

/**
 * Parse command line argument
 * 
 * @param argc number of parameters
 * @param argv string containing the command line arguments
 * @param inst model instance
*/
void parse_command_line(int argc, char** argv, instance *inst);

//TODO use only euclidean distance or also manhattan (no sqrt, probably faster)
/**
 * Compute the euclidean distance between two points
 * 
 * @param i index of first point
 * @param j index of second point
 * @param inst model instance
 * @return euclidean distance value
*/
double dist(int i, int j, instance* inst);

/**
 * Compute the distances between all nodes
 * 
 * @param inst model instance
*/
void compute_distances(instance* inst);

/**
 * Get cost of the edge between nodes i and j
 * 
 * @param i index of first node
 * @param j index of second node
 * @param inst model instance
 * @return cost of the relative edge
*/
double get_cost(int i, int j, instance* inst);

/**
 * Update model solution if the input cost is better
 * 
 * @param z solution cost
 * @param solution candidate solution
 * @param inst model instance
*/
void update_solution(double z, int* solution, instance* inst);

/**
 * Check if the input solution is a valid solution for the tsp.
 * Print a debug error if the solution is not valid.
 * 
 * @param solution array representing the solution
 * @param length length of the solution array
*/
void check_solution(int* solution, int length);

/**
 * Free memory regarding the tsp instance
 * 
 * @param inst tsp model instance
*/
void free_instance(instance* inst);

/**
 * Print info about the input tsp instance
 * 
 * @param inst input tsp instance
*/
void print_instance(instance* inst);

/**
 * Print info about command line arguments
*/
void print_help();
