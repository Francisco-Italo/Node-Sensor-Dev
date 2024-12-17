/*
 * uart.c
 *
 *  Created on: 10 de dez de 2024
 *      Author: italo
 */

#include <msp430.h>
#include "uart.h"

volatile char rx_buffer[BUFFER_SIZE];
volatile unsigned char rx_index, c;

void uart_setup(void)
{
    // Configure UART pins
    P1SEL0 |= BIT4 | BIT5;                    // set 2-UART pin as second function
    P1SEL1 &= ~(BIT4 | BIT5);

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
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void uart_transmit_char(char c) {
    while (!(UCA0IFG & UCTXIFG));  // Wait for TX buffer to be ready
    UCA0TXBUF = c;
}

void uart_transmit_string(volatile char *str) {
    while (*str) {
        uart_transmit_char(*str++);
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        c = UCA0RXBUF;
        rx_buffer[rx_index] = c;

        if(c == '\n')
        {
            rx_buffer[rx_index+1] = '\0';
            rx_index = 0;
            if(rx_buffer[4] == 'G' && rx_buffer[5] == 'A')
            {
                uart_transmit_string(rx_buffer);
            }

            P1OUT ^= BIT0;
        }
        else
        {
            rx_index++;
        }
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}
// End of file
