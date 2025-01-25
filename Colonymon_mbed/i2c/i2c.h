/*
 * i2c.h
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#ifndef I2C_I2C_H_
#define I2C_I2C_H_

/**
 * General use
 */
unsigned char slaveAddress;
unsigned char RX_Data[6];
unsigned char TX_Data[2];
unsigned char RX_ByteCtr;
unsigned char TX_ByteCtr;

enum rw_bit
{
    READ=0,
    WRITE
};

/**
 * Prototypes
 */
void i2c_init(void);
void i2c_trans(unsigned char, enum rw_bit, unsigned char);

#endif /* I2C_I2C_H_ */
