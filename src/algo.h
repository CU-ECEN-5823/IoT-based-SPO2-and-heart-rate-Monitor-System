/*
 * algo.h
 *
 *  Created on: 2 Dec 2021
 *      Author: nihalt
 */

#ifndef SRC_ALGO_H_
#define SRC_ALGO_H_

#include <stdint.h>
#include <stdbool.h>

bool checkForBeat(int32_t sample);
int16_t averageDCEstimator(int32_t *p, uint16_t x);
int16_t lowPassFIRFilter(int16_t din);
int32_t mul16(int16_t x, int16_t y);


#endif /* SRC_ALGO_H_ */
