//PLOT HEADER

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tsp.h"

/**
 * Create .dat file for plotting
 * 
 * @param output_path path corresponding to dat file to be created
 * @param inst tsp model instance
*/
void create_dat_file(char* output_path, instance* inst);

/**
 * Create plot figure path
 * 
 * @param output_path path corresponding to png file to be created
 * @param model_name name of the current model
*/
void create_plot_path(char* output_path, char* model_name);

/**
 * Create command txt file for gnuplot
 * 
 * @param output_path path corresponding to output command file
 * @param inst tsp model instance
*/
void create_command(char* output_path, instance* inst);

/**
 * Plot figure
 * 
 * @param inst tsp model instance
*/
void plot_graph(instance* inst);