#include "plot.h"
#include "utils.h"

void create_dat_file(char* output_path, instance* inst) {
    // define path for dat file
    char data_path[100];
    create_path(data_path, "data", inst->name, "dat");

    // Write data to dat file
    FILE* fp = fopen(data_path, "w");
    if(fp == NULL)
        print_error("Error during .dat file creation");

    //write start node at the beginning
    //fprintf(fp, "%f %f\n", inst->points[inst->best_sol[0]].x, inst->points[inst->best_sol[0]].y);
    //write all the nodes twice
    for(int i=0; i<inst->nnodes; ++i) {
        int succ_node = inst->best_sol[i];
        fprintf(fp, "%f %f\n", inst->points[i].x, inst->points[i].y);
        fprintf(fp, "%f %f\n\n", inst->points[succ_node].x, inst->points[succ_node].y);
    }
    //write start node at the end
    //fprintf(fp, "%f %f\n", inst->points[inst->best_sol[0]].x, inst->points[inst->best_sol[0]].y);

    fclose(fp);
    // set data path
    strcpy(output_path, data_path);
}

void create_plot_path(char* output_path, char* model_name) {
    char figure_path[200];
    create_path(figure_path, "plot", model_name, "png");

    // set png file path
    strcpy(output_path, figure_path);
}

void create_command(char* output_path, instance* inst) {
    //dat file path
    char data_path[100];
    create_dat_file(data_path, inst);

    //command file
    char command_path[100];
    create_path(command_path, "plot", "command", "txt");

    //figure file path
    char figure_path[100];
    create_plot_path(figure_path, inst->name);

    //try to create plot directory
    mkdir("../plot", 0777);

    //populate command file
    FILE* cf = fopen(command_path, "w");
    if(cf == NULL)
        print_error("Error creating command file for gnuplot");

    fprintf(cf, "set terminal pngcairo size 1280,720\n");
    fprintf(cf, "set output '%s'\n\n", figure_path);

    fprintf(cf, "set title 'My Plot'\n");
    fprintf(cf, "set xlabel 'X-axis Label'\n");
    fprintf(cf, "set ylabel 'Y-axis Label'\n\n");

    fprintf(cf, "set style line 1 lc rgb '#ff0000' lt 1 lw 2 pt 7 pi -1 ps 1.5\n");
    fprintf(cf, "set pointintervalbox 2\n\n");

    fprintf(cf, "plot '%s' with linespoints ls 1", data_path);

    fclose(cf);

    strcpy(output_path, command_path);
}

void plot_solution(instance* inst) {
    char command_path[100];
    create_command(command_path, inst);

    char command[200];
    strcpy(command, "gnuplot ");
    strcat(command, command_path);

    //execute gnuplot
    system(command);
}