//******************************************************************************
//  MSP430FR24xx Demo - Timer1_A3, Toggle P1.0, CCR0 Cont Mode ISR, 32KHz ACLK
//
//  Description: Toggle P1.0 using software and TA1_0 ISR. Timer1_A is
//  configured for continuous mode, thus the timer overflows when TAR counts
//  to CCR0. In this example, CCR0 is loaded with 50000.
//  ACLK = TACLK = 32768Hz, MCLK = SMCLK  = default DCO = ~1MHz
//
//          MSP430FR2433
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |           P1.0|-->LED
//
//  Wei Zhao
//  Texas Instruments Inc.
//  Jan 2014
//  Built with IAR Embedded Workbench v6.20 & Code Composer Studio v6.0.1
//******************************************************************************

#include <msp430.h>
#include "clk/clk.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                     // Stop WDT

    clock_setup();

    // Configure GPIO
    P1DIR |= BIT0;                                // P1.0 output

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Timer0_A0 setup
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR; // SMCLK, Up mode, Clear timer
    TA0CCR0 = 50 - 1; // 50 µs interval (assuming SMCLK = 1 MHz)
    TA0CCTL0 = CCIE; // Enable CCR0 interrupt

    __bis_SR_register(LPM3_bits | GIE);          // Enter LPM0 w/ interrupt
    __no_operation();                            // For debug
}

// Timer A1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    P1OUT ^= BIT0;
}
