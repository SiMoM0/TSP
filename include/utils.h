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
 * Read inputs and create tsp instance
 * 
 * @param inst instance to be defined
*/
void read_input(instance *inst);

/**
 * Parse command line argument
 * 
 * @param argc number of parameters
 * @param argv string containing the command line arguments
 * @param inst model instance
*/
void parse_command_line(int argc, char** argv, instance *inst);

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

/**
 * Plot graph using gnuplot
 * 
 * @param inst input model instance
*/
void plot(instance* inst);
