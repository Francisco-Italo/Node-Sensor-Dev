#include "msp_conf.h"
#include "uart/uart.h"
#include "clk/clk.h"

unsigned char TXData[] = "Hello Embedded World\n\r";

int main(void)
{
    WDTCTL = WDT_ADLY_1000;                 // WDT 1000ms, ACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                              // to activate previously configured port settings
    P1OUT &= ~BIT0;                           // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                            // Set P1.0 to output direction

    clk_setup();
    uart_setup();

    __bis_SR_register(LPM3_bits | GIE);     // Enter LPM3
    __no_operation();                       // For debug

}

// Watchdog Timer interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) WDT_ISR (void)
#else
#error Compiler not supported!
#endif
{
    ser_output(TXData);
    P1OUT ^= BIT0;
}
// End of file
