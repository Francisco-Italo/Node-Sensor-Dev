#include <msp430.h>
#include "clk/clk.h"

void gpio_init_unused(void);

int main(void)
{
    WDTCTL = WDT_ADLY_1000;                 // WDT 1000ms, ACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Port Configuration all un-used pins to output low
    gpio_init_unused();
    clk_setup();

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
    P1OUT ^= BIT0;                          // Toggle P1.0 (LED) every 1s
}

void gpio_init_unused(void)
{
    P1OUT = 0x00;P2OUT = 0x00;P3OUT = 0x00;
    P1DIR = 0xff;P2DIR = 0xff;P3DIR = 0xff;
}
// End of file
