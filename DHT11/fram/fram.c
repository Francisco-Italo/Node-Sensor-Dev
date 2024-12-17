/*
 * fram.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#include "../msp_conf.h"
#include "fram.h"

int *FRAM_write_ptr;

void FRAMWrite (int value)
{
    SYSCFG0 = FRWPPW | PFWP;
    *FRAM_write_ptr = value;
    SYSCFG0 = FRWPPW | DFWP | PFWP;
}

