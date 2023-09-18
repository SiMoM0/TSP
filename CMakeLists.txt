# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project(TSP)

# header files
include_directories(include)

# CPLEX paths
set(CPLEX_PATH /opt/ibm/ILOG/CPLEX_Studio2211/cplex)
set(CPLEX_LIB ${CPLEX_PATH}/lib/x86-64_linux/static_pic)
# CONCORDE lib
set(CONCORDELIB /opt/concorde)

#PROVA PERCHè NON FUNZIONA
#SET (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ldl")
#SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ldl")

# Add an executable
add_executable(main src/main.c src/utils.c src/heuristics.c src/metaheuristics.c src/plot.c src/solver.c src/benders.c src/callbacks.c src/matheuristics.c)
#add_executable(${PROJECT_NAME} ${tsp_SRC})

# CPLEX
include_directories(${CPLEX_PATH}/include/ilcplex)
# Concorde
include_directories(${CONCORDELIB} ${CONCORDELIB}/INCLUDE)

add_definitions(-DIL_STD)
set (CPLEX_LINKER_FLAGS -lcplex -lm -lpthread -ldl)


# math library
target_link_libraries(main PRIVATE m)
# CPLEX libraries
target_link_libraries(main PRIVATE ${CPLEX_LINKER_FLAGS})
target_link_libraries(main PRIVATE -L${CPLEX_LIB})
# Concorde
target_link_libraries(main PRIVATE ${CONCORDELIB}/concorde.a)
