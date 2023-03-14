#include "utils.h"

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

void print_error(const char *err) {
    printf("\n\n ERROR: %s \n\n", err);
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
			if(strncmp(token1, "ATT", 3) != 0)
				print_error("Format error:  only EDGE_WEIGHT_TYPE == ATT implemented so far!!!!!!");
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

	if(verbose >= 10)
		print_instance(inst);

	fclose(fin);
}

void parse_command_line(int argc, char** argv, instance *inst) {
	// default instance attributes
	strcpy(inst->input_file, "NULL");
	inst->randomseed = 10;
	inst->timelimit = INFINITY;
	inst->verbose = 0;
	inst->nnodes = 0;
	inst->zbest = -1;

	// flag for help command
    int help = 0;

	for(int i=1; i<argc; i++) {
		//TODO use duplicates or not (?)
		if(strcmp(argv[i],"-file") == 0) {strcpy(inst->input_file,argv[++i]); continue;} 			// input file
		if(strcmp(argv[i],"-input") == 0) {strcpy(inst->input_file,argv[++i]); continue;} 			// input file
		if(strcmp(argv[i],"-f") == 0) {strcpy(inst->input_file,argv[++i]); continue;} 				// input file
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

double dist(int i, int j, instance* inst) {
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

	for(int i=0; i<inst->nnodes; ++i) {
		for(int j=0; j<inst->nnodes; ++j) {
			inst->distances[i*inst->nnodes + j] = dist(i, j, inst);
		}
	}
}

double get_cost(int i, int j, instance* inst) {
	return inst->distances[i*inst->nnodes + j];
}

void update_solution(double z, int* solution, instance* inst) {
	if(inst->zbest < 0.0 || z < inst->zbest) {
		inst->zbest = z;
		memcpy(inst->best_sol, solution, inst->nnodes * sizeof(int));
	}
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
	printf("time limit: 		%lf\n", inst->timelimit);
	printf("random seed: 		%d\n", inst->randomseed);
	printf("verbose: 		%d\n\n", inst->verbose);
}

void print_help(){
	printf("AVAILABLE PARAMETERS TO MODIFY:\n");
	printf("-file <file's path>       Path of the file to solve the problem\n");
    printf("-time_limit <time>        The time limit in seconds\n");
	printf("-seed <seed>              The seed for random number generation\n");
	printf("-verbose <level>          It displays detailed processing information on the screen\n");
}
