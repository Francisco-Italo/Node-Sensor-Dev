/**
 * Excitation+ (E+)
 * Excitation- (E-)
 * Output/Signal/Amplifier+ (O/S/A+)
 * Output/Signal/Amplifier- (O/S/A-)
 */

#include "msp_conf.h"
#include "clk/clk.h"
#include "fram/fram.h"
#include "uart/uart.h"
#include "hx711/hx711.h"

volatile char i;

int main(void)
{
    WDTCTL = WDT_ADLY_1000;                 // WDT 1000ms, ACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    clk_setup();

    uart_init();
    scale_init();

    SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
    for(i = 16; i > 0; --i)
    {
        tare = tare+scale_read();
    }
    tare >>= 4;
    SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)

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
    SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
    for(i = 16; i > 0; --i)
    {
        y = y+scale_read();
        y = y-tare;
    }
    y >>= 4;
    SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)

    uart_out(&y, sizeof(y));

    SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
    y ^= y;
    SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)
}
// End of file
