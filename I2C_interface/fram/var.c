/*
 * var.c
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#include "var.h"

#ifdef __TI_COMPILER_VERSION__
#pragma PERSISTENT(_accel_pck)
struct accel_pck _accel_pck = {0,0,0};
#pragma PERSISTENT(_co2_pck)
struct co2_pck _co2_pck = {0,0};
#elif __IAR_SYSTEMS_ICC__
_persistent struct accel_pck _accel_pck = {0,0,0};
_persistent struct co2_pck _co2_pck = {0,0};
#else
// Port the following variable to an equivalent persistent functionality for the specific compiler being used
struct accel_pck _accel_pck = {0,0,0};
struct co2_pck _co2_pck = {0,0};
#endif
