/*
 * uart.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include "../msp_conf.h"
#include "uart.h"

void uart_setup(void)
{
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
// End of file
