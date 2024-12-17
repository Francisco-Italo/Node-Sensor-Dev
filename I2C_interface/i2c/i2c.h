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
unsigned char slaveAddress, rw;
unsigned char RX_Data[6];
unsigned char TX_Data[2];
unsigned char RX_ByteCtr;
unsigned char TX_ByteCtr;

/**
 * Prototypes
 */
void i2c_conf(void);
void i2c_trans(unsigned char, unsigned char, unsigned char);

#endif /* I2C_I2C_H_ */
