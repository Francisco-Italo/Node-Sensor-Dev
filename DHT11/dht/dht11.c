/*
 * dht11.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include "../msp_conf.h"
#include "dht11.h"

#define DHT11 BIT6
#define SENS_IN P1IN
#define SENS_OUT P1OUT
#define SENS_DIR P1DIR
#define SENS_INT P1IFG

volatile char elapsed_time;
unsigned char hum_int,hum_decimals,tmp_int,tmp_decimals;

/**
 * DHT11 comm. signal processing
 */
unsigned char recv_sig(void)
{
    char val = 0;
    char bit_cnt;

    for(bit_cnt=0;bit_cnt<8;bit_cnt++) // for humidity first byte
    {
        while(!(SENS_IN&DHT11)); // skip the lower 50 uS part

        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
        }
        while((SENS_IN&DHT11)==DHT11);
        if(elapsed_time > 5)
        {
            val |= 0x01;
        }

        val <<= 1;
    }

    return (val>>=1);   // Return corrected value
}

/**
 * DHT11 handler
 */
int dht11(void)
{
    unsigned char parity;
    __delay_cycles(1100000);        // Sensor needs time between readings

    SENS_DIR |= DHT11; // Makes the pin OUTput
    SENS_OUT &= ~DHT11;// Start condition generation for DHT11
    __delay_cycles(20000); // delay ~20 milisecond for each scan

    SENS_OUT |= DHT11;// Start condition generation for DHT11
    SENS_DIR &= ~DHT11; // Makes the pin input
    SENS_INT &= ~DHT11; // Clears pin flag, if high

    while((SENS_IN&DHT11) == DHT11); // wait till the slave pulls the pin low

    /**
     * DHT11 has responded
     *
    */
    do
    {
        elapsed_time++; // check if the elapsed time = 80 uS
    }
    while(!(SENS_IN&DHT11));

    if(elapsed_time <= 10)
    {
        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
        }
        while((SENS_IN&DHT11)==DHT11);

        if(elapsed_time <=10)// check if the elapsed time = 80 uS
        {
            // Integer part of humidity
            hum_int = recv_sig();
            // Decimal part of humidity
            hum_decimals = recv_sig();
            // Integer part of temperature
            tmp_int = recv_sig();
            // Decimal part of temperature
            tmp_decimals = recv_sig();
            // Parity checksum
            parity = recv_sig();
        }
    }

    return (((hum_int + hum_decimals + tmp_int + tmp_decimals) == parity) ? 1 : 0);
}
// End of file
