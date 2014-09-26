/*
 * swmr-testdata.h
 *
 *  Created on: 24 Sep 2014
 *      Author: up45
 */

#ifndef SWMR_TESTDATA_H_
#define SWMR_TESTDATA_H_

#include "hdf5.h"

const unsigned int swmr_iterations = 20;

// 2 small images of 3x4
unsigned int swmr_testdata[2][3][4] = { {
                                           {     1,    45,   343,   675},
                                           {   643,  2143,   875,    34},
                                           {   842,   482,  5000,  3762}
                                                                        },{
                                           {     2,  4346, 54322,  2354},
                                           { 35521,  3462, 34333,  7674},
                                           {    41,  5532,   563,    84}
                                       } };

typedef struct Image_t{
  hsize_t dims[2];
  unsigned long long framenumber;
  unsigned int * pdata;
} Image_t;

#endif /* SWMR_TESTDATA_H_ */
