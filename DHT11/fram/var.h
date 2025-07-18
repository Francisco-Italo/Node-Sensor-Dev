/*
 * var.h
 *
 *  Created on: 7 de jan de 2025
 *      Author: italo
 */

#ifndef FRAM_VAR_H_
#define FRAM_VAR_H_

struct temp_hum_pck
{
    unsigned char hum_int,hum_decimals;
    unsigned char tmp_int,tmp_decimals;
    unsigned int checksum;
};

extern struct temp_hum_pck _pck;

#endif /* FRAM_VAR_H_ */
