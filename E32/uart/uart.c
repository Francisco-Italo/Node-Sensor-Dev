/*
 * uart.c
 *
 *  Created on: 3 de dez de 2024
 *      Author: italo
 */

#include <msp430.h>
#include "uart.h"

#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz
#define DCODIV_1MHZ   30
//#define DECIMAL_PLACES 2

void uart_init(void)
{
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + DCODIV_1MHZ;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    Software_Trim();                        // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                              // to activate previously configured port settings
    // Configure UART pins
    P1SEL0 |= BIT4 | BIT5;                    // set 2-UART pin as second function

    // Configure UART
    UCA0CTLW0 |= UCSWRST;                     // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;

    // Baud Rate calculation (infos in UG table 22-5)
    UCA0BR0 = 6;                              // 1000000/16/9600 = 6.51
    UCA0BR1 = 0;
    UCA0MCTLW = 0x2000 | UCOS16 | UCBRF_8;      // 1000000/16/9600 - INT(1000000/16/9600)=0.51
                                                // UCBRFx = int (0.51*16) = 8; 16 bcz of UCOS16
                                                // UCBRSx value = 0x20
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
}

void uart_out(const void *data, unsigned char length)
{
  const char* buff = (const char*)data;
  char i;
  for(i = 0; i < length; ++i)
  {
      while(!(UCA0IFG&UCTXIFG));
      UCA0TXBUF = buff[i];
      //_delay_cycles(2000);        // Logic analyzer crutch
  }
}

void int_conv(unsigned int regVal, unsigned char *str)
{
  unsigned char i = 0, j = 0;
  unsigned char aux[] = {'0','0','0','0','0','\0'};

  do
  {
    aux[i++] = regVal % 10 + '0';
    regVal /= 10;
  }
  while(regVal > 0);

  while(i--)
  {
    str[j++] = aux[i];
  }
  str[j] = '\0';
}

void serial_out(volatile unsigned char* str)
{
    while(*str)
    {
        while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = *str++;
    }
}

/*void float_conv(float value, volatile unsigned char *str)
{
    long integer_part = (long)value; // Extract integer part
    float fractional_part = value - integer_part; // Extract fractional part
    if (value < 0)
    {
        *str++ = '-'; // Add minus sign for negative values
        integer_part = -integer_part;
        fractional_part = -fractional_part;
    }

    // Convert integer part to string
    unsigned char temp[12];
    unsigned char i = 0;
    do
    {
        temp[i++] = (integer_part % 10) + '0';
        integer_part /= 10;
    }while (integer_part);

    while (i > 0)
    {
        *str++ = temp[--i];
    }

    // Add decimal point
    *str++ = '.';

    // Convert fractional part to string
    char j;
    long digit;
    for (j = 0; j < DECIMAL_PLACES; j++)
    {
        fractional_part *= 10;
        digit = (long)fractional_part;
        *str++ = digit + '0';
        fractional_part -= digit;
    }

    *str= '\0'; // Null-terminate the string
}*/

void Software_Trim(void)
{
    unsigned int oldDcoTap = 0xffff;
    unsigned int newDcoTap = 0xffff;
    unsigned int newDcoDelta = 0xffff;
    unsigned int bestDcoDelta = 0xffff;
    unsigned int csCtl0Copy = 0;
    unsigned int csCtl1Copy = 0;
    unsigned int csCtl0Read = 0;
    unsigned int csCtl1Read = 0;
    unsigned int dcoFreqTrim = 3;
    unsigned char endLoop = 0;

    do
    {
        CSCTL0 = 0x100;                         // DCO Tap = 256
        do
        {
            CSCTL7 &= ~DCOFFG;                  // Clear DCO fault flag
        }while (CSCTL7 & DCOFFG);               // Test DCO fault flag

        __delay_cycles((unsigned int)3000 * MCLK_FREQ_MHZ);// Wait FLL lock status (FLLUNLOCK) to be stable
                                                           // Suggest to wait 24 cycles of divided FLL reference clock
        while((CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)) && ((CSCTL7 & DCOFFG) == 0));

        csCtl0Read = CSCTL0;                   // Read CSCTL0
        csCtl1Read = CSCTL1;                   // Read CSCTL1

        oldDcoTap = newDcoTap;                 // Record DCOTAP value of last time
        newDcoTap = csCtl0Read & 0x01ff;       // Get DCOTAP value of this time
        dcoFreqTrim = (csCtl1Read & 0x0070)>>4;// Get DCOFTRIM value

        if(newDcoTap < 256)                    // DCOTAP < 256
        {
            newDcoDelta = 256 - newDcoTap;     // Delta value between DCPTAP and 256
            if((oldDcoTap != 0xffff) && (oldDcoTap >= 256)) // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim--;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM0)) | (dcoFreqTrim<<4);
            }
        }
        else                                   // DCOTAP >= 256
        {
            newDcoDelta = newDcoTap - 256;     // Delta value between DCPTAP and 256
            if(oldDcoTap < 256)                // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim++;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM0)) | (dcoFreqTrim<<4);
            }
        }

        if(newDcoDelta < bestDcoDelta)         // Record DCOTAP closest to 256
        {
            csCtl0Copy = csCtl0Read;
            csCtl1Copy = csCtl1Read;
            bestDcoDelta = newDcoDelta;
        }

    }while(endLoop == 0);                      // Poll until endLoop == 1

    CSCTL0 = csCtl0Copy;                       // Reload locked DCOTAP
    CSCTL1 = csCtl1Copy;                       // Reload locked DCOFTRIM
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
}
// End of file
