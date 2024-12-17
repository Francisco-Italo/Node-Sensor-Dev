/*
 * fram.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include "../msp_conf.h"
#include "fram.h"

void FRAMWrite (void)
{
    unsigned char i;
    SYSCFG0 = FRWPPW | PFWP;
    for (i = 0; i < 128; i++)
    {
        *FRAM_write_ptr++ = data;
    }
    SYSCFG0 = FRWPPW | DFWP | PFWP;
}
// End file
