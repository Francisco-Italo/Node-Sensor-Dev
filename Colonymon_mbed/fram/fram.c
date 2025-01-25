/*
 * fram.c
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#include <fram/fram.h>

#ifdef __TI_COMPILER_VERSION__
#pragma PERSISTENT(_pck)
union node_t _pck = {0};
#elif __IAR_SYSTEMS_ICC__
_persistent union node_t _pck = {0};
#else
// Port the following variable to an equivalent persistent functionality for the specific compiler being used
union node_t _pck = {0};
#endif
