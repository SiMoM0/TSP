# Travelling Salesman Problem (TSP) Optimization

Repository for the Operations Research 2 project regarding the TSP optimization, UniPD 2022/23.

The goal of the project is to solve the Travelling Salesman Problem with different approaches such as constructive heuristics, refinement heuristics, metaheuristics ...

## Project Structure

    TSP
    ├── CMakeLists.txt          Project CMakeLists
    ├── README.md               Project README
    ├── build                   Generated by CMake
    │   ├── ...
    │   ├── main                Main executable file
    ├── data                    TSP data file
    │   └── att48.tsp           Example .tsp file
    ├── include                 Header files
    │   ├── ...
    │   ├── tsp.h
    │   └── utils.h
    ├── plot                    Plot files
    │   └── att48.png           Example plot file
    └── src                     Source files
        ├── ...
        ├── main.c
        └── utils.c

## Requirements

* [Gnuplot](http://www.gnuplot.info/):  a portable command-line driven graphing utility

## Installation

* Create the *build* directory using `mkdir build`
* Navigate into the *build* directory with `cd build`
* Run the command `cmake ..` to configure the project
* Execute the `make` command to build the project

## Usage

Input data are in `.tsp` from [TSPLIB](http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/). Some of them are already stored in the `data` directory.

Run the main program with the following command and set the desired parameters:

```
./main -file <filepath> -time_limit <tl> -seed <s> -verbose <v>
```

Example: solve the TSP regarding the **att48** instance stored in the `../data/att48.tsp` file, running the command:

```
./main -file ../data/att48.tsp -time_limit 60 -seed 10 -verbose 10
```
