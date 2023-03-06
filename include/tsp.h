// TSP model

#pragma once

//hard-wired parameters
#define XSMALL		  		  1e-5 		// 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON		  		  1e-9		// 1e-9		// very small numerical tolerance

/**
 * TSP instance
*/
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
