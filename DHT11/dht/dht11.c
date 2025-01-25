/*
 * dht11.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include "../msp_conf.h"
#include "../fram/var.h"
#include "dht11.h"

#define DHT11 BIT4
#define SENS_IN P2IN
#define SENS_OUT P2OUT
#define SENS_DIR P2DIR
#define SENS_INT P2IFG

unsigned char elapsed_time;
//unsigned int loop_cnt;

/**
 * DHT11 comm. signal processing
 */
unsigned char recv_signal(void)
{
    char val = 0;
    char bit_cnt;

    for(bit_cnt=8;bit_cnt>0;--bit_cnt) // for humidity first byte
    {
        elapsed_time = 0;
        // skip the lower 50 uS part
        while(!(SENS_IN&DHT11))
        {
            elapsed_time++;
            /*if(elapsed_time > 200)
            {
                return 1;
            }*/
        }

        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
            /*if(elapsed_time > 200)
            {
                return 1;
            }*/
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
 * DHT11 read function
 */
int dht11_read(void)
{
    /**
     * Starting protocol
     */
    __delay_cycles(1100000);        // Sensor needs time between readings
    //loop_cnt = 0;

    SENS_DIR |= DHT11; // Makes the pin OUTput
    SENS_OUT &= ~DHT11;// Start condition generation for DHT11
    __delay_cycles(25000); // delay ~20 milisecond for each scan

    SENS_OUT |= DHT11;// Start condition generation for DHT11
    __delay_cycles(30); // Approximately 18 us
    SENS_DIR &= ~DHT11; // Makes the pin input

    // wait till the slave pulls the pin low
    while((SENS_IN&DHT11) == DHT11);

    /**
     * DHT11 read function
     *
    */
    unsigned char parity;

    do
    {
        elapsed_time++; // check if the elapsed time = 80 uS
        /*if(elapsed_time > 200)
        {
            return 1;
        }*/
    }
    while(!(SENS_IN&DHT11));

    if(elapsed_time <= 10)
    {
        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
            /*if(elapsed_time > 200)
            {
                return 1;
            }*/
        }
        while((SENS_IN&DHT11)==DHT11);

        if(elapsed_time <=10)// check if the elapsed time = 80 uS
        {
            SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
            // Integer part of humidity
            _pck.hum_int = recv_signal();
            // Decimal part of humidity
            _pck.hum_decimals = recv_signal();
            // Integer part of temperature
            _pck.tmp_int = recv_signal();
            // Decimal part of temperature
            _pck.tmp_decimals = recv_signal();
            // Parity checksum
            parity = recv_signal();
            _pck.checksum = _pck.hum_int + _pck.hum_decimals + _pck.tmp_int + _pck.tmp_decimals;
            SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }

    return ((_pck.checksum == parity) ? 0 : 1);
}
// End of file
