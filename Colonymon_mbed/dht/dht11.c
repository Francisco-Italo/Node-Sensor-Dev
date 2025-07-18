/*
 * dht11.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include <msp430.h>
#include <fram/fram.h>
#include "dht11.h"

#define DHT11 BIT0
#define SENS_IN P1IN
#define SENS_OUT P1OUT
#define SENS_DIR P1DIR
#define SENS_INT P1IFG

unsigned char elapsed_time,bit_cnt;
unsigned char hum_int,hum_dec,tmp_int,tmp_dec,parity;

/**
 * DHT11 function to read humidity, temperature and checksum
 */
int dht11(void)
{
    /**
     * Starting protocol
     */
    SENS_DIR |= DHT11; // Makes the pin OUTput
    SENS_OUT &= ~DHT11;// Start condition generation for DHT11
    __delay_cycles(18000); // Delay ~18 ms for each scan

    SENS_OUT |= DHT11;// Start condition generation for DHT11
    __delay_cycles(40); // Delay ~40 us
    SENS_DIR &= ~DHT11; // Makes the pin input

    ///////////////// DHT11 has responded /////////////////////////////
    do
    {
        elapsed_time++; 
    }
    while ((SENS_IN&DHT11) == 0);
    
    if(elapsed_time <= 10)      // Check if the elapsed time = 80 uS
    {
        elapsed_time = 0;
        do 
        {
            elapsed_time++;
        }
        while ((SENS_IN&DHT11)==DHT11);
        
        if(elapsed_time <=10)       // Check if the elapsed time = 80 uS
        {
            ///////// humidity integer/////////////
            for(bit_cnt=8;bit_cnt>0;--bit_cnt) 
            {
                while ((P1IN&DHT11)==0); // skip the lower 50 uS part
                
                elapsed_time = 0;
                do
                {
                    elapsed_time++; // check if the elapsed time = 80 uS
                }
                while ((P1IN&DHT11)==DHT11);
                
                if(elapsed_time>5)
                {
                    hum_int |= 0x01;
                }
                
                hum_int <<= 1;
            }
            ///////// humidity decimal/////////////
            for(bit_cnt=8;bit_cnt>0;--bit_cnt) 
            {
                while ((P1IN&DHT11)==0); // skip the lower 50 uS part
                
                elapsed_time = 0;
                do
                {
                    elapsed_time++; // check if the elapsed time = 80 uS
                }
                while ((P1IN&DHT11)==DHT11);
                
                if(elapsed_time>5)
                {
                    hum_dec |= 0x01;
                }
                hum_dec <<= 1;
            }
            ///////// temperature integer/////////////
            for(bit_cnt=8;bit_cnt>0;--bit_cnt) 
            {
                while ((P1IN&DHT11)==0); // skip the lower 50 uS part
                
                elapsed_time = 0;
                do
                {
                    elapsed_time++; // check if the elapsed time = 80 uS
                }
                while ((P1IN&DHT11)==DHT11);
                
                if(elapsed_time>5)
                {
                    tmp_int |= 0x01;
                }
                
                tmp_int <<= 1;
            }
            ///////// temperature decimal/////////////
            for(bit_cnt=8;bit_cnt>0;--bit_cnt) 
            {
                while ((P1IN&DHT11)==0); // skip the lower 50 uS part
                
                elapsed_time = 0;
                do
                {
                    elapsed_time++; // check if the elapsed time = 80 uS
                }
                while ((P1IN&DHT11)==DHT11);
                
                if(elapsed_time>5)
                {
                    tmp_dec |= 0x01;
                }
                
                tmp_dec <<= 1;
            }
            /////////Parity/////////////
            for(bit_cnt=8;bit_cnt>0;--bit_cnt) 
            {
                while ((P1IN&DHT11)==0); // skip the lower 50 uS part
                
                elapsed_time = 0;
                do
                {
                    elapsed_time++; // check if the elapsed time = 80 uS
                }
                while ((P1IN&DHT11)==DHT11);
                
                if(elapsed_time>5)
                {
                    parity |= 0x01;
                }
                parity <<= 1;
            }
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
    SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
    _pck.sensor_data._dht_pck.hum_int = (hum_int>>1); _pck.sensor_data._dht_pck.hum_decimals = (hum_dec>>1);
    _pck.sensor_data._dht_pck.tmp_int = (tmp_int>>1); _pck.sensor_data._dht_pck.tmp_decimals = (tmp_dec>>1);
    _pck.sensor_data._dht_pck.checksum = (parity>>1);
    SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)

    return ((hum_int+hum_dec+tmp_int+tmp_dec == _pck.sensor_data._dht_pck.checksum) ? STATUS_OK : STATUS_CHECKSUM_ERR);
}
// End of file
