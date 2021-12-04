/*
 * MAX_30101.h
 *
 *  Created on: 1 Dec 2021
 *      Author: nihalt
 */

#ifndef SRC_MAX_30101_H_
#define SRC_MAX_30101_H_

#include <stddef.h>
#include <stdint.h>

void MAX_30101_Init();
void MAX_30101_Get_Reg_Val (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data);
void MAX_30101_ShutDown();
void MAX_30101_PowerUp();

#endif /* SRC_MAX_30101_H_ */
