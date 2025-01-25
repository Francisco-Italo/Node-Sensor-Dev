/*
 * var.c
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#include "var.h"

#ifdef __TI_COMPILER_VERSION__
#pragma PERSISTENT(_pck)
struct temp_hum_pck _pck = {0,0,0,0,0};
#elif __IAR_SYSTEMS_ICC__
_persistent struct temp_hum_pck _pck = {0,0,0,0,0};
#else
// Port the following variable to an equivalent persistent functionality for the specific compiler being used
struct temp_hum_pck _pck = {0,0,0,0,0};
#endif
