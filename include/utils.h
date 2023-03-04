// UTILS FILE

#pragma once

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

//hard-wired parameters
#define XSMALL		  		  1e-5 		// 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON		  		  1e-9		// 1e-9		// very small numerical tolerance

typedef struct {
    //input data
    int nnodes;
    double *xcoord;
    double *ycoord;

    //parameters
    int randomseed;
    double timelimit;
    char input_file[1000];
    int verbose;
} instance;

//inline functions
inline int imax(int i1, int i2) { return ( i1 > i2 ) ? i1 : i2; } 
inline double dmin(double d1, double d2) { return ( d1 < d2 ) ? d1 : d2; } 
inline double dmax(double d1, double d2) { return ( d1 > d2 ) ? d1 : d2; } 

void print_error(const char *err);
void debug(const char *err);   

void read_input(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst);
void free_instance(instance* inst);

void plot(instance* inst);
