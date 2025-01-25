/*
 * i2c.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#include <msp430.h>
#include "i2c.h"

//-------------------------------------------------------------------------------------------------//
// I2C related routines
//-------------------------------------------------------------------------------------------------//
/**
 * Configuration of I2C module
 */
void i2c_init(void)
{
    // I2C pins configuration
    P1SEL0 |= BIT2 | BIT3;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                    // Software reset enabled

    UCB0CTLW0 |= UCMODE_3 | UCMST;          // I2C mode, Master mode, SMCLK
    UCB0CTLW0 |= UCSSEL__SMCLK | UCSYNC;    // Use SMCLK as clock source, sync
    UCB0CTLW1 |= UCASTP_2;                  // Automatic STOP generated by UCB0TBCNT
    UCB0BR0 = 0x000A;                       // baudrate = SMCLK / 10 = ~100 kHz

    UCB0CTLW0 &= ~UCSWRST;                   // Disable SW reset
}
/**
 * I2C transaction function
 */
void i2c_trans(unsigned char addr, enum rw_bit rw, unsigned char byteCtr)
{
    while(UCB0CTL1&UCTXSTP);

    UCB0CTLW0 |= UCSWRST;
    UCB0I2CSA = addr;                   // Slave address
    UCB0TBCNT = byteCtr;
    UCB0CTLW0 &= ~UCSWRST;
    UCB0IE |= UCTXIE0 | UCRXIE0 | UCBCNTIE;

    (rw) ? (UCB0CTL1 |= UCTR):(UCB0CTL1 &= ~UCTR);  // Clear -> receiver / Set -> transmitter
    UCB0CTL1 |= UCTXSTT;

    __bis_SR_register(LPM3_bits|GIE);   // Enter LPM0 w/ interrupt
}
/**
 * UCB0 ISR
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCIB0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
      case USCI_I2C_UCNACKIFG:break;
      case USCI_I2C_UCRXIFG0:
          RX_Data[--RX_ByteCtr] = UCB0RXBUF;  // Get received byte
          break;
      case USCI_I2C_UCTXIFG0:
          UCB0TXBUF = TX_Data[--TX_ByteCtr];  // Passes byte to transmit buffer
          break;
      case USCI_I2C_UCBCNTIFG:
          UCB0CTL1 |= UCTXSTP;
          __bic_SR_register_on_exit(LPM3_bits);  // Exit LPM3
          break;
  }
}
// End of file
