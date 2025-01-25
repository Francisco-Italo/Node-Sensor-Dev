/*
 * var.h
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#ifndef FRAM_VAR_H_
#define FRAM_VAR_H_

/**
 * Data collect variables
 */
struct accel_pck
{
    unsigned int xAxis;
    unsigned int yAxis;
    unsigned int zAxis;
};

extern struct accel_pck _accel_pck;

struct co2_pck
{
    unsigned int co2Lvl;
    unsigned int tvocLvl;
};

extern struct co2_pck _co2_pck;

#endif /* FRAM_VAR_H_ */
