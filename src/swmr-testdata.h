/*
 * swmr-testdata.h
 *
 *  Created on: 24 Sep 2014
 *      Author: up45
 */

#ifndef SWMR_TESTDATA_H_
#define SWMR_TESTDATA_H_

#include <stdint.h>

const unsigned int swmr_iterations = 20;

// 2 small images of 3x4
const unsigned long long swmr_testdata_rows = 3;
const unsigned long long swmr_testdata_cols = 4;
const uint32_t swmr_testdata[2][3][4] =  { {
                                           {     1,    45,   343,   675},
                                           {   643,  2143,   875,    34},
                                           {   842,   482,  5000,  3762}
                                                                        },{
                                           {     2,  4346, 54322,  2354},
                                           { 35521,  3462, 34333,  7674},
                                           {    41,  5532,   563,    84}
                                       } };

#endif /* SWMR_TESTDATA_H_ */
