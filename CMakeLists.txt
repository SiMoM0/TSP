# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project(TSP)

# CPLEX paths
set(CPLEX_PATH /opt/ibm/ILOG/CPLEX_Studio2211/cplex)
set(CPLEX_LIB ${CPLEX_PATH}/lib/x86-64_linux/static_pic)

# Add an executable
add_executable(main src/main.c src/utils.c src/heuristics.c src/metaheuristics.c src/plot.c src/solver.c)

# header files
include_directories(include)
# CPLEX
include_directories(${CPLEX_PATH}/include/ilcplex)

add_definitions(-DIL_STD)
set (CPLEX_LINKER_FLAGS -lcplex -lm -lpthread)

# math library
target_link_libraries(main PRIVATE m)
# CPLEX libraries
target_link_libraries(main PRIVATE ${CPLEX_LINKER_FLAGS})
target_link_libraries(main PRIVATE -L${CPLEX_LIB})