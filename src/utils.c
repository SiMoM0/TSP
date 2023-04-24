#include "utils.h"
#include "plot.h"

int imax(int i1, int i2) {
	return (i1 > i2) ? i1 : i2;
}

int imin(int i1, int i2) {
	return (i1 < i2) ? i1 : i2;
}

double dmax(double d1, double d2) {
	return (d1 > d2) ? d1 : d2;
}

double dmin(double d1, double d2) {
	return (d1 < d2) ? d1 : d2;
}

double nint(double x){
	return (long) (x + 0.5);
}

void print_error(const char *err) {
    printf("\n\nERROR: %s \n\n", err);
    fflush(NULL);
    exit(1);
}

void debug(const char *err) {
    printf("\nDEBUG: %s \n", err);
    fflush(NULL);
}

void create_path(char* output_path, char* dir, char* filename, char* ext) {
	//check input parameters
	if(dir == NULL || filename == NULL || ext == NULL)
		print_error("Missing values for path generation");

	//path to be created
	char path[200] = "../";

	//add directory name
	strcat(path, dir);
	strcat(path, "/");
	
	//add file name
	strcat(path, filename);
	
	//add extension
	strcat(path, ".");
	strcat(path, ext);

	//update output_path
	strcpy(output_path, path);
}

void parse_model(instance *inst) {
	//open input file in read mode
    FILE *fin = fopen(inst->input_file, "r");
	if(fin == NULL)
		print_error("Input file not found!");
	
	inst->nnodes = -1;
    int verbose = inst->verbose;

	char line[180];
	char *par_name;   
	char *token1;
	char *token2;
	
	int active_section = 0; // =1 NODE_COORD_SECTION, =2 DEMAND_SECTION, =3 DEPOT_SECTION 
	
	int do_print = (verbose >= 1000);

	while(fgets(line, sizeof(line), fin) != NULL) {
		if (verbose >= 2000) {
			printf("%s",line);
			fflush(NULL);
		}

		// skip empty lines
		if(strlen(line) <= 1)
			continue;

	    par_name = strtok(line, " :");
		//TODO:change verbose format
		if(verbose >= 3000) {
			printf("parameter \"%s\" ",par_name);
			fflush(NULL);
		}

		if(strncmp(par_name, "NAME", 4) == 0) {
			active_section = 0;
			token1 = strtok(NULL, " : ");
			//remove \n at the end of the string
			token1[strcspn(token1, "\n")] = 0;
			strcpy(inst->name, token1);
			continue;
		}

		if(strncmp(par_name, "COMMENT", 7) == 0) {
			active_section = 0;   
			token1 = strtok(NULL, "");
			continue;
		}   
		
		if(strncmp(par_name, "TYPE", 4) == 0) {
			token1 = strtok(NULL, " :");  
			if(strncmp(token1, "TSP",3) != 0)
				print_error("Format error: only TYPE == TSP implemented so far!!!!!!"); 
			active_section = 0;
			continue;
		}
		
		if(strncmp(par_name, "DIMENSION", 9) == 0) {
			if(inst->nnodes >= 0)
				print_error("Repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);
			if(do_print)
				printf(" ... nnodes %d\n", inst->nnodes);
			inst->points = (point *) calloc(inst->nnodes, sizeof(point));
			active_section = 0;
			continue;
		}

		if(strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0) {
			token1 = strtok(NULL, " :");
			if(strncmp(token1, "EUC_2D", 6) == 0){
				inst->edge_weight_type = EUC_2D;
			} else if(strncmp(token1, "MAN_2D", 6) == 0) {
            	inst->edge_weight_type = MAN_2D;
			} else if(strncmp(token1, "ATT", 3) == 0) {
            	inst->edge_weight_type = ATT;
			} else {
				print_error("Format error:  Wrong edge weight type!");
			}
			if(inst->nnodes >= 0)
				inst->best_sol = (int *) calloc(inst->nnodes, sizeof(int));
			active_section = 0;
			continue;
		}            
		
		if(strncmp(par_name, "NODE_COORD_SECTION", 18) == 0) {
			if(inst->nnodes <= 0)
				print_error("DIMENSION section should appear before NODE_COORD_SECTION section");
			active_section = 1;   
			continue;
		}
		
		if(strncmp(par_name, "EOF", 3) == 0) {
			active_section = 0;
			break;
		}

        if(active_section == 1) {
            // within NODE_COORD_SECTION
			int i = atoi(par_name) - 1;
			if(i < 0 || i >= inst->nnodes)
				print_error("unknown node in NODE_COORD_SECTION section");
			token1 = strtok(NULL, " ");
			token2 = strtok(NULL, " ");
			inst->points[i].x = atof(token1);
			inst->points[i].y = atof(token2);
			if(do_print)
				printf("node %4d at coordinates ( %15.7lf , %15.7lf )\n", i+1, inst->points[i].x, inst->points[i].y);
			continue;
		}
		
		printf("Final active section %d\n", active_section);
		print_error("Wrong format for the current simplified parser!!!!!!!!!");  
	}                

	if(verbose >= 1)
		print_instance(inst);

	fclose(fin);
}

void parse_command_line(int argc, char** argv, instance *inst) {
	if(argc<2){
        printf("Usage: %s -help for the available commands\n", argv[0]);
        exit(1);
    }
	
	// default instance attributes
	strcpy(inst->input_file, "NULL");
	strcpy(inst->solver, "GREEDY");
	inst->randomseed = 10;
	inst->timelimit = DEFAULT_TIME_LIM;
	inst->verbose = 1;
	inst->nnodes = 0;
	inst->zbest = -1;

	// flag for help command
    int help = 0;

	for(int i=1; i<argc; i++) {
		//TODO use duplicates or not (?)
		if(strcmp(argv[i],"-file") == 0) {strcpy(inst->input_file,argv[++i]); continue;} 			// input file
		if(strcmp(argv[i],"-input") == 0) {strcpy(inst->input_file,argv[++i]); continue;} 			// input file
		if(strcmp(argv[i],"-f") == 0) {strcpy(inst->input_file,argv[++i]); continue;} 				// input file
		if(strcmp(argv[i], "-solver") == 0) {strcpy(inst->solver, argv[++i]); continue;}				// solver
		if(strcmp(argv[i],"-time_limit") == 0) {inst->timelimit = atof(argv[++i]); continue;}		// total time limit
		if(strcmp(argv[i],"-seed") == 0) {inst->randomseed = abs(atoi(argv[++i])); continue;} 		// random seed
        if(strcmp(argv[i], "-verbose") == 0) {inst->verbose = atoi(argv[++i]); continue;}			// verbose parameter
        if(strcmp(argv[i], "-v") == 0) {inst->verbose = atoi(argv[++i]); continue;}					// verbose parameter
		if(strcmp(argv[i],"-help") == 0) {help = 1; continue;} 										// help
		if(strcmp(argv[i],"--help") == 0) {help = 1; continue;} 									// help
		help = 1;
    }      

	//print help instruction if verbose >= 10
	if(inst->verbose >= 10) {
		printf("Enter -help or --help for help\n\n");
	}        
	
	//show all the available parameters
	if(help){
		print_help();
        exit(0);
	}
}

//Computation of pseudo_euc_dist (Minkowski distance)
double pseudo_euc_dist(int i, int j, instance* inst) {
	double dist; 

	double dx = inst->points[i].x - inst->points[j].x;
	double dy = inst->points[i].y - inst->points[j].y;

	double rij = sqrt((dx*dx + dy*dy) / 10.0);
	double tij = nint(rij);
	if (tij<rij) {  
		dist = tij + 1;
    } else {
        dist = tij;
    }

	return dist;
}

double man2d_dist(int i, int j, instance* inst) {
	double dx = inst->points[i].x - inst->points[j].x;
	double dy = inst->points[i].y - inst->points[j].y;

	return abs(dx) + abs(dy);
}

double euc2d_dist(int i, int j, instance* inst) {
	double dx = inst->points[i].x - inst->points[j].x;
	double dy = inst->points[i].y - inst->points[j].y;

	return sqrt(dx*dx + dy*dy);
}

void compute_distances(instance* inst) {
	//check if nnodes is greater than 0
	if(inst->nnodes <= 0)
		print_error("No nodes in the graph");

	//allocate memory for the costs array
	inst->distances = (double *) calloc(inst->nnodes*inst->nnodes, sizeof(double));

	if(inst->edge_weight_type == MAN_2D){
		for(int i=0; i<inst->nnodes; ++i) {
			for(int j=0; j<inst->nnodes; ++j) {
				inst->distances[i*inst->nnodes + j] = man2d_dist(i, j, inst);
			}	
		}
	} else if(inst->edge_weight_type == ATT){
		for(int i=0; i<inst->nnodes; ++i) {
			for(int j=0; j<inst->nnodes; ++j) {
				inst->distances[i*inst->nnodes + j] = pseudo_euc_dist(i, j, inst);
			}	
		}
	} else {
		//the default is the 2d euclidean
		for(int i=0; i<inst->nnodes; ++i) {
			for(int j=0; j<inst->nnodes; ++j) {
				inst->distances[i*inst->nnodes + j] = euc2d_dist(i, j, inst);
			}	
		}
	}
	
	
	
}

double get_cost(int i, int j, instance* inst) {
	return inst->distances[i*inst->nnodes + j];
}

int xpos(int i, int j, instance* inst) {
	if(i == j)
		print_error("i==j in xpos");
	if(i > j)
		return xpos(j, i, inst);
	int pos = i * inst->nnodes + j - ((i+1) * (i+2)) / 2;
	return pos;
}

void reverse_path(int* solution, int start, int end) {
	int prev = start;
    int node = solution[start];
	int last_node = solution[end];
    while(node != last_node) {
        int next_node = solution[node];
        solution[node] = prev;
        prev = node;
        //update node
        node = next_node;
    }
}

void update_solution(double z, int* solution, instance* inst) {
	if(inst->zbest < 0.0 || z < inst->zbest) {
		inst->zbest = z;
		memcpy(inst->best_sol, solution, inst->nnodes * sizeof(int));
	}
}

void check_solution(int* solution, int length) {
	int* check_array = calloc(length, sizeof(int));

	for(int i=0; i<length; ++i) {
		check_array[i] = 0;
	}

	for(int i=0; i<length; ++i) {
		check_array[solution[i]]++;
	}

	for(int i=0; i<length; ++i) {
		if(check_array[i] != 1) {
			debug("TSP solution not valid");
			break;
		}
	}

	free(check_array);
	//debug("TSP solution is valid");
}

void shake(instance* inst, int* input_solution) {
    double z = 0;

    int idx1 = rand() % inst->nnodes;
    int idx2=idx1;
    int idx3=idx1;
    while(idx2==idx1 || abs(idx1-idx2)<=1){    // no same node and not successor or predecessor idx1
        idx2=rand() % inst->nnodes;
        
    }
    while(idx3==idx1 || idx3==idx2 || abs(idx1-idx3)<=1 || abs(idx2-idx3)<=1){
        idx3=rand() % inst->nnodes;
    }
    //put them in order
    if(idx1>idx2){
        swap(&idx1, &idx2);
    }
    if(idx1>idx3){
        swap(&idx1, &idx3);
    }   
    if(idx2>idx3){
        swap(&idx2, &idx3);
    }
    //printf("idx1 = %d, idx2 = %d, idx3 = %d\n", idx1, idx2, idx3);

    int* tour = calloc(inst->nnodes, sizeof(int));
    int node = input_solution[0];
    for(int i=0; i<inst->nnodes; ++i) {
        tour[i] = node;
        node = input_solution[node];
    }

    //choose the nodes that are involved in the swap edges procedure
    int nodeA = tour[idx1];
    int nodeB = input_solution[nodeA];
    int nodeC = tour[idx2];
    int nodeD = input_solution[nodeC];
    int nodeE = tour[idx3];
    int nodeF = input_solution[nodeE];

    //printf("EDGES [%d-%d] [%d-%d] [%d-%d]\n", nodeA, nodeB, nodeC, nodeD, nodeE, nodeF);

    //case 7
    input_solution[nodeA] = nodeD; //succ of A is D
    input_solution[nodeE] = nodeB; //succ of E is B
    input_solution[nodeC] = nodeF; //succ of C is F

    //printf("NEW EDGES [%d-%d] [%d-%d] [%d-%d]\n", nodeA, solution[nodeA], nodeC, solution[nodeC], nodeE, solution[nodeE]);
    
    check_solution(input_solution, inst->nnodes);

    //debug_plot(inst, solution);

    
    //compute the new cost 
    for(int i=0; i<inst->nnodes; i++) {
        z += get_cost(i, input_solution[i], inst);
    }

    //printf("New cost after shake: %f\n", z);

    free(tour);
}

void swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void free_instance(instance *inst) {
	free(inst->points);
	free(inst->distances);
	free(inst->best_sol);
}

void print_instance(instance* inst) {
	printf("INSTANCE INFO\n");
	printf("input file: 		%s\n", inst->input_file);
	printf("number of nodes: 	%d\n", inst->nnodes);
	printf("solver:			%s\n", inst->solver);
	printf("time limit: 		%lf\n", inst->timelimit);
	printf("random seed: 		%d\n", inst->randomseed);
	printf("verbose:		%d\n", inst->verbose);
	if(inst->edge_weight_type == EUC_2D){
		printf("edge weight type:	EUC_2D\n\n");
	} else if(inst->edge_weight_type == MAN_2D){
		printf("edge weight type:	MAN_2D\n\n");
	}else if(inst->edge_weight_type == ATT){
		printf("edge weight type:	ATT\n\n");
	}

}

void print_help(){
	printf("AVAILABLE PARAMETERS TO MODIFY:\n");
	printf("-file <filepath>       Path of the file to solve the problem\n");
	printf("-solver <solver>	  Selected solver for the problem\n");
    printf("-time_limit <time>        The time limit in seconds\n");
	printf("-seed <seed>              The seed for random number generation\n");
	printf("-verbose <level>          It displays detailed processing information on the screen\n\n");

	printf("AVAILABLE SOLVERS:\n");
	printf("GREEDY					Greedy algorithm\n");
	printf("GRASP					Grasp algorithm\n");
	printf("EXTRA_MIL				Extra Mileage algorithm\n");
	printf("GREEDY_ITER				Greedy iterative algorithm\n");
	printf("GRASP_ITER				Grasp iterative algorithm\n");
	printf("GREEDY_2OPT				Greedy and 2-opt algorithm\n");
	printf("GRASP_2OPT				Grasp and 2-opt algorithm\n");
	printf("EXTRA_MIL_2OPT				Extra Mileage and 2-opt algorithm\n");
	printf("TABU_SEARCH				Tabu Search algorithm\n");
	printf("VNS					Variable Neighbor Search algorithm\n");
	printf("CPLEX				Standard Cplex solver\n");
}

void debug_plot(instance* inst, int* input_solution) {
	memcpy(inst->best_sol, input_solution, inst->nnodes * sizeof(int));
    plot_solution(inst);
    printf("ENTER to continue");
    getchar();
}

void progressbar(int progress, int total) {
    int width = 50;
    float progress_ratio = (float) progress / (float) total;
    int bar_progress = (int)(progress_ratio * width);

    printf("[");
    for(int i=0; i<width; ++i) {
        if(i<bar_progress) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] %d%%\t%d/%ds\r", (int)(progress_ratio * 100), progress, total);
    fflush(stdout);

	//clear current line
	printf("\033[2K");
}
