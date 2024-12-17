/*
 * sensors.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include "i2c.h"
#include "sensors.h"

/**
 * Data collect variables
 */
/*unsigned int xAccel;
unsigned int yAccel;
unsigned int zAccel;*/

unsigned int co2Lvl;
unsigned int tvocLvl;

/**
 * MPU-6050 & CCS811 relevant register addresses
 */
/*static const unsigned char MPU_ADDR     = 0x68;      // ADO = GND. Else, 0x69
static const unsigned char PWR_MGMT_1   = 0x6B;
static const unsigned char ACCEL_XOUT_H = 0x3B;*/

static const unsigned char CCS811_ADDR = 0x5A;
static const unsigned char CCS811_MEAS_MODE = 0x01;
static const unsigned char CCS811_ALG_RESULT_DATA = 0x02;

//-------------------------------------------------------------------------------------------------//
// Accelerometer routines
//-------------------------------------------------------------------------------------------------//
/**
 * MPU setup
 */
/*void acc_setup()
{
    // Wake up the MPU-6050
    slaveAddress = MPU_ADDR;
    TX_Data[1] = PWR_MGMT_1;
    TX_Data[0] = 0x08;      // Set 8 MHz clock; disable temperature sensor
    TX_ByteCtr = 2;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
    __delay_cycles(32000);                // According to datasheet, hold ~30 ms
                                            // Because of gyroscope based clock oscillator
}*/
/**
 * Communication with accelerometer device
 */
/*void acc_comm()
{
    // I2C Transaction
    slaveAddress = MPU_ADDR;            // Register pointing
    TX_Data[0] = ACCEL_XOUT_H;          // First address of the set
    TX_ByteCtr = 1;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
    RX_ByteCtr = 6;                     // Read six bytes of data
    rw = 0;
    i2c_trans(slaveAddress, rw, RX_ByteCtr);

    xAccel  = RX_Data[5] << 8 | RX_Data[4];
    yAccel  = RX_Data[3] << 8 | RX_Data[2];
    zAccel  = RX_Data[1] << 8 | RX_Data[0];
}*/
//-------------------------------------------------------------------------------------------------//
// Gases sensor routines
//-------------------------------------------------------------------------------------------------//
/**
 * CCS811 setup
 */
void gas_setup()
{
    slaveAddress = CCS811_ADDR;
    TX_Data[1] = CCS811_MEAS_MODE;
    TX_Data[0] = 0x10;                  // Put CCS to normal mode, no interrupt enable
    TX_ByteCtr = 2;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
}
/**
 * Communication with gases sensor
 */
void gas_comm()
{
    // Read data
    slaveAddress = CCS811_ADDR;
    TX_Data[0] = CCS811_ALG_RESULT_DATA;
    TX_ByteCtr = 1;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
    RX_ByteCtr = 8;                  // Reading operation of environment data register
    rw = 0;
    i2c_trans(slaveAddress, rw, RX_ByteCtr);

    co2Lvl = (RX_Data[0] << 8 | RX_Data[1]);
    tvocLvl = (RX_Data[2] << 8 | RX_Data[3]);
}
// End of file
