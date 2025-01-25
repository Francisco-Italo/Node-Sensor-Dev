/*
 * sensors.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include <fram/fram.h>

#include <msp430.h>
#include "i2c.h"
#include "sensors.h"

/**
 * MPU-6050 & CCS811 relevant register addresses
 */
static const unsigned char MPU_ADDR     = 0x68;      // ADO = GND. Else, 0x69
static const unsigned char CCS811_ADDR  = 0x5A;

//-------------------------------------------------------------------------------------------------//
// Accelerometer routines
//-------------------------------------------------------------------------------------------------//
/**
 * MPU setup
 */
void acc_setup()
{
    static const unsigned char PWR_MGMT_1   = 0x6B;

    // Wake up the MPU-6050
    slaveAddress = MPU_ADDR;
    TX_Data[1] = PWR_MGMT_1;
    TX_Data[0] = 0x08;      // Set 8 MHz clock; disable temperature sensor
    TX_ByteCtr = 2;
    i2c_trans(slaveAddress, WRITE, TX_ByteCtr);
    __delay_cycles(32000);                // According to datasheet, hold ~30 ms
                                            // Because of gyroscope based clock oscillator
}
/**
 * Communication with accelerometer device
 */
void acc_comm()
{
    static const unsigned char ACCEL_XOUT_H = 0x3B;

    // I2C Transaction
    slaveAddress = MPU_ADDR;            // Register pointing
    TX_Data[0] = ACCEL_XOUT_H;          // First address of the set
    TX_ByteCtr = 1;
    i2c_trans(slaveAddress, WRITE, TX_ByteCtr);
    RX_ByteCtr = 6;                     // Read six bytes of data
    i2c_trans(slaveAddress, READ, RX_ByteCtr);

    SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
    _pck.sensor_data._mpu_pck.xAxis  = RX_Data[5] << 8 | RX_Data[4];
    _pck.sensor_data._mpu_pck.yAxis  = RX_Data[3] << 8 | RX_Data[2];
    _pck.sensor_data._mpu_pck.zAxis  = RX_Data[1] << 8 | RX_Data[0];
    SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)

}
//-------------------------------------------------------------------------------------------------//
// Gases sensor routines
//-------------------------------------------------------------------------------------------------//
/**
 * CCS811 setup
 */
void gas_setup()
{
    static const unsigned char CCS811_APP_START = 0xF4;
    static const unsigned char CCS811_MEAS_MODE = 0x01;

    slaveAddress = CCS811_ADDR;
    TX_Data[0] = CCS811_APP_START;  // Change sensor from boot to application mode
    TX_ByteCtr = 1;
    i2c_trans(slaveAddress, WRITE, TX_ByteCtr);
    //slaveAddress = CCS811_ADDR;
    TX_Data[1] = CCS811_MEAS_MODE;
    TX_Data[0] = 0x10;                  // Put CCS to normal mode, no interrupt enable
    TX_ByteCtr = 2;
    i2c_trans(slaveAddress, WRITE, TX_ByteCtr);
}
/**
 * Communication with gases sensor
 */
void gas_comm()
{
    static const unsigned char CCS811_ALG_RESULT_DATA = 0x02;

    // Read data
    slaveAddress = CCS811_ADDR;
    TX_Data[0] = CCS811_ALG_RESULT_DATA;
    TX_ByteCtr = 1;
    i2c_trans(slaveAddress, WRITE, TX_ByteCtr);
    RX_ByteCtr = 4;                  // Reading operation of environment data register
    i2c_trans(slaveAddress, READ, RX_ByteCtr);

    SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
    _pck.sensor_data._ccs_pck.co2Lvl = (RX_Data[3] << 8 | RX_Data[2]);
    _pck.sensor_data._ccs_pck.tvocLvl = (RX_Data[1] << 8 | RX_Data[0]);
    SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)
}
// End of file
