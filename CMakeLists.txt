# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (TSP)

include_directories(include)

# Add an executable
add_executable(main src/main.c src/utils.c src/heuristics.c src/plot.c)

# math library
target_link_libraries(main PRIVATE m)