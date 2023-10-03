// TSP model

#pragma once

//hard-wired parameters
#define XSMALL		  		  1e-5 		// 1e-4*	// tolerance used to decide ingerality of 0-1 var.s
#define EPSILON		  		  1e-9		// 1e-9		// very small numerical tolerance
#define DEFAULT_TIME_LIM      60       // 1 min
/**
 * Point
*/
typedef struct point{
    double x;
    double y;
} point;

/**
 * Distance types
*/
typedef enum {
    EUC_2D,     //euclidean distance
    MAN_2D,     //manhattan distance
    ATT         //pseudo-euclidean
} edge_weight_type;


/**
 * TSP instance
*/
typedef struct instance {
    //input data
    int nnodes;
    point* points;
    double* distances;
    int integer_costs;

    //parameters
    char name[100];
    int randomseed;
    double timelimit;
    char input_file[1000];
    int verbose;
    edge_weight_type edge_weight_type;

    char solver[100];
    int cplex; //if cplex = 1, use cplex.
    int ncols;
    
    //solution
    double zbest;
    int* best_sol;
    double exec_time;
} instance;
