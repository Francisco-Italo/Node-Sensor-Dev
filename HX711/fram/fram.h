/*
 * var.h
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#ifndef FRAM_FRAM_H_
#define FRAM_FRAM_H_

// Statically-initialized variable
#ifdef __TI_COMPILER_VERSION__
#pragma PERSISTENT(y)
volatile unsigned long y = 0;
#pragma PERSISTENT(tare)
unsigned long tare = 0;
#elif __IAR_SYSTEMS_ICC__
_persistent volatile unsigned long y = 0;
_persistent unsigned long tare;
#else
// Port the following variable to an equivalent persistent functionality for the specific compiler being used
volatile unsigned long y = 0;
unsigned long tare = 0;
#endif

#endif /* FRAM_FRAM_H_ */
