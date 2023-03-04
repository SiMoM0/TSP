// MAIN FILE

#include "utils.h"

int main(int argc, char** argv) {

    if(argc<2){
        printf("Usage: %s -help for help\n", argv[0]);
        exit(1);
    }

    //tsp instance
    instance inst;
    
    parse_command_line(argc,argv, &inst);

    read_input(&inst);

    plot(&inst);

    free_instance(&inst);
    
    return 0;
}
