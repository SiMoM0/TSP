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

void read_input(instance *inst) {
    FILE *fin = fopen(inst->input_file, "r");
	if ( fin == NULL ) print_error(" input file not found!");
	
	inst->nnodes = -1;
    int verbose = inst->verbose;

	char line[180];
	char *par_name;   
	char *token1;
	char *token2;
	
	int active_section = 0; // =1 NODE_COORD_SECTION, =2 DEMAND_SECTION, =3 DEPOT_SECTION 
	
	int do_print = (verbose >= 1000 );

	while(fgets(line, sizeof(line), fin) != NULL) {
		if (verbose >= 2000 ) { printf("%s",line); fflush(NULL); }
		if ( strlen(line) <= 1 ) continue; // skip empty lines
	    par_name = strtok(line, " :");
		if(verbose >= 3000) {
			printf("parameter \"%s\" ",par_name);
			fflush(NULL);
		}

		if(strncmp(par_name, "NAME", 4) == 0) {
			active_section = 0;
			continue;
		}

		if(strncmp(par_name, "COMMENT", 7) == 0) {
			active_section = 0;   
			token1 = strtok(NULL, "");
			continue;
		}   
		
		if(strncmp(par_name, "TYPE", 4) == 0) {
			token1 = strtok(NULL, " :");  
			if ( strncmp(token1, "TSP",3) != 0 ) print_error(" format error:  only TYPE == TSP implemented so far!!!!!!"); 
			active_section = 0;
			continue;
		}
		

		if(strncmp(par_name, "DIMENSION", 9) == 0) {
			if ( inst->nnodes >= 0 ) print_error(" repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :");
			inst->nnodes = atoi(token1);
			if ( do_print ) printf(" ... nnodes %d\n", inst->nnodes);
			inst->points = (point *) calloc(inst->nnodes, sizeof(point));
			active_section = 0;
			continue;
		}

		if(strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0) {
			token1 = strtok(NULL, " :");
			if ( strncmp(token1, "ATT", 3) != 0 ) print_error(" format error:  only EDGE_WEIGHT_TYPE == ATT implemented so far!!!!!!"); 
			active_section = 0;
			continue;
		}            
		
		if(strncmp(par_name, "NODE_COORD_SECTION", 18) == 0) {
			if ( inst->nnodes <= 0 ) print_error(" ... DIMENSION section should appear before NODE_COORD_SECTION section");
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
			if ( i < 0 || i >= inst->nnodes ) print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, " ");
			token2 = strtok(NULL, " ");
			inst->points[i].x = atof(token1);
			inst->points[i].y = atof(token2);
			if ( do_print ) printf(" ... node %4d at coordinates ( %15.7lf , %15.7lf )\n", i+1, inst->points[i].x, inst->points[i].y); 
			continue;
		}
		
		printf(" final active section %d\n", active_section);
		print_error(" ... wrong format for the current simplified parser!!!!!!!!!");  
	}                

	fclose(fin);
}

void parse_command_line(int argc, char** argv, instance *inst) {
	// default instance attributes
	strcpy(inst->input_file, "NULL");
	inst->randomseed = 10;
	inst->timelimit = INFINITY;
	inst->verbose = 0;
	inst->nnodes = 0;

	// flag for help command
    int help = 0;

	for(int i=1; i<argc; i++) {
		//TODO use duplicates or not (?)
		if(strcmp(argv[i],"-file") == 0) { strcpy(inst->input_file,argv[++i]); continue;} 			// input file
		if(strcmp(argv[i],"-input") == 0) { strcpy(inst->input_file,argv[++i]); continue;} 			// input file
		if(strcmp(argv[i],"-f") == 0) { strcpy(inst->input_file,argv[++i]); continue;} 				// input file
		if(strcmp(argv[i],"-time_limit") == 0) { inst->timelimit = atof(argv[++i]); continue;}		// total time limit
		if(strcmp(argv[i],"-seed") == 0) { inst->randomseed = abs(atoi(argv[++i])); continue;} 		// random seed
        if(strcmp(argv[i], "-verbose") == 0) {inst->verbose = atoi(argv[++i]); continue;}			// verbose parameter
        if(strcmp(argv[i], "-v") == 0) {inst->verbose = atoi(argv[++i]); continue;}					// verbose parameter
		if(strcmp(argv[i],"-help") == 0) { help = 1; continue;} 									// help
		if(strcmp(argv[i],"--help") == 0) { help = 1; continue;} 									// help
		help = 1;
    }      

	//print all the main parameters of instance if verbose>=10 
	if((inst->verbose >= 10)) {
	    // print current parameters
		print_instance(inst);
		printf("\nEnter -help or --help for help\n");
	}        
	
	//show all the parameters to modify
	if(help){
		print_help();
        exit(0);
	}
}

void free_instance(instance *inst) {
	free(inst->points);
}

void print_instance(instance* inst) {
	printf("INSTANCE INFO\n");
	printf("	input file: %s\n", inst->input_file);
	printf("	number of nodes: %d\n", inst->nnodes);
	printf("	time limit: %lf\n", inst->timelimit);
	printf("	random seed: %d\n", inst->randomseed);
	printf("	verbose: %d\n", inst->verbose);
}

void print_help(){
	printf("AVAILABLE PARAMETERS TO MODIFY:\n");
	printf("-file <file's path>       Path of the file to solve the problem\n");
    printf("-time_limit <time>        The time limit in seconds\n");
	printf("-seed <seed>              The seed for random number generation\n");
	printf("-verbose <level>          It displays detailed processing information on the screen\n");
}

void plot(instance* inst) {
	//TODO: change function to generate and use a command.txt file for plotting
    //define output path for dat file
    char output_path[100];
    strcpy(output_path, inst->input_file);
	output_path[strlen(output_path)-3] = 0;
    strcat(output_path, "dat");

    printf("Output path: %s, len = %ld\n", output_path, strlen(output_path));

    // Write data to output dat file
    FILE* fp = fopen(output_path, "w");
    for(int i = 0; i < inst->nnodes; ++i) {
        fprintf(fp, "%f %f\n", inst->points[i].x, inst->points[i].y);
    }
    fclose(fp);

    //set command to execute plot
    char command[200];
    strcpy(command, "gnuplot -persist -e \"plot '");
    strcat(command, output_path);
	/*	ls -> linecolor
		lc -> linecolor
		lt -> linetype
		ps -> pointsize
		pt -> pointtype
		#ff0000 -> red in HEX
	*/
    strcat(command, "' with linespoints ls 1 lc rgb '#ff0000' lt 1 pt 7 pi -1 ps 2\"");

	printf("Gnuplot command: %s\n", command);

    system(command);
}
