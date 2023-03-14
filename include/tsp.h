// TSP model

#pragma once

//hard-wired parameters
#define XSMALL		  		  1e-5 		// 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON		  		  1e-9		// 1e-9		// very small numerical tolerance

/**
 * Point
*/
typedef struct point{
    double x;
    double y;
} point;

/**
 * TSP instance
*/
typedef struct instance {
    //input data
    int nnodes;
    point* points;
    //TODO better to define an edge struct (?)
    double* distances;

    //parameters
    char name[100];
    int randomseed;
    double timelimit;
    char input_file[1000];
    int verbose;

    //solution
    double zbest;
    int* best_sol;
} instance;
