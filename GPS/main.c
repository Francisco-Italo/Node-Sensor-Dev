#include <msp430.h> 
#include "clk/clk.h"
#include "uart/uart.h"

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                                  // to activate previously configured port settings
    clock_setup();
    uart_setup();

    P1DIR |= BIT0; P1OUT &= ~(BIT0);

     __bis_SR_register(LPM3_bits|GIE);         // Enter LPM3, interrupts enabled
    __no_operation();
}
// End of file
