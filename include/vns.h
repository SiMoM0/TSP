#pragma once

#include "utils.h"

typedef enum {
    OPT3_CASE_0,
    OPT3_CASE_1,
    OPT3_CASE_2,
    OPT3_CASE_3,
    OPT3_CASE_4,
    OPT3_CASE_5,
    OPT3_CASE_6,     
    OPT3_CASE_7
} opt_cases;

/**
 * Variable neighborhood search VNS
 * 
 * @param inst model instance
 * @returns an integer which represents the status code
*/
int VNS(instance* inst);