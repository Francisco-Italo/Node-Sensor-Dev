/*
 * dht11.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include <msp430.h>
#include "../fram/fram.h"
#include "dht11.h"

#define DHT11 BIT0
#define SENS_IN P1IN
#define SENS_OUT P1OUT
#define SENS_DIR P1DIR
#define SENS_INT P1IFG

unsigned char elapsed_time;
unsigned int loop;

/**
 * DHT11 comm. signal processing
 */
signed char recv_signal(void)
{
    char val = 0;
    char bit_cnt;

    for(bit_cnt=8;bit_cnt>0;--bit_cnt) // for humidity first byte
    {
        loop = 200;
        // skip the lower 50 uS part
        while(!(SENS_IN&DHT11))
        {
            loop--;
            if(!loop)
            {
                return STATUS_TIMEOUT;
            }
        }

        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
            if(elapsed_time > 200)
            {
                return STATUS_TIMEOUT;
            }
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
     * Starting ptotocol
     */
    __delay_cycles(1100000);        // Sensor needs time between readings

    SENS_DIR |= DHT11; // Makes the pin OUTput
    SENS_OUT &= ~DHT11;// Start condition generation for DHT11
    __delay_cycles(25000); // delay ~20 milisecond for each scan

    SENS_OUT |= DHT11;// Start condition generation for DHT11
    __delay_cycles(30); // Approximately 18 us
    SENS_DIR &= ~DHT11; // Makes the pin input

    loop = 10000;
    // wait till the slave pulls the pin low
    while((SENS_IN&DHT11) == DHT11)
    {
        loop--;
        if(!loop)
        {
            return STATUS_TIMEOUT;
        }
    }

    /**
     * DHT11 read function
     *
    */
    unsigned char parity;

    elapsed_time = 0;
    do
    {
        elapsed_time++; // check if the elapsed time = 80 uS
        if(elapsed_time > 200)
        {
            return STATUS_TIMEOUT;
        }
    }
    while(!(SENS_IN&DHT11));

    if(elapsed_time <= 10)
    {
        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
            if(elapsed_time > 200)
            {
                return STATUS_TIMEOUT;
            }
        }
        while((SENS_IN&DHT11)==DHT11);

        if(elapsed_time <=10)// check if the elapsed time = 80 uS
        {
            signed char buff;
            SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
            // Integer part of humidity
            buff = recv_signal();
            if(buff != STATUS_TIMEOUT) _pck.sensor_data._dht_pck.hum_int = buff;
            else _pck.sensor_data._dht_pck.hum_int = 50;
            // Decimal part of humidity
            buff = recv_signal();
            if(buff != STATUS_TIMEOUT) _pck.sensor_data._dht_pck.hum_decimals = buff;
            // Integer part of temperature
            buff = recv_signal();
            if(buff != STATUS_TIMEOUT) _pck.sensor_data._dht_pck.tmp_int = buff;
            else _pck.sensor_data._dht_pck.tmp_int = 30;
            // Decimal part of temperature
            buff = recv_signal();
            if(buff != STATUS_TIMEOUT) _pck.sensor_data._dht_pck.tmp_decimals = buff;
            // Parity checksum
            parity = recv_signal();
            _pck.sensor_data._dht_pck.checksum = _pck.sensor_data._dht_pck.hum_int +
                    _pck.sensor_data._dht_pck.hum_decimals +
                    _pck.sensor_data._dht_pck.tmp_int +
                    _pck.sensor_data._dht_pck.tmp_decimals;
            SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)
        }
        else
        {
            return STATUS_TIMEOUT;
        }
    }
    else
    {
        return STATUS_TIMEOUT;
    }

    return ((_pck.sensor_data._dht_pck.checksum == parity) ? STATUS_OK : STATUS_CHECKSUM_ERR);
}
// End of file
