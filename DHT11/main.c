#include "msp_conf.h"
#include "fram/var.h"
#include "uart/uart.h"
#include "dht/dht11.h"
#include "clk/clk.h"

#define INTERVAL  3                         // 5 mins -> 300
volatile unsigned int wdt_cnt;              // WDT interrupt time counter

/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDT_ADLY_1000;                   // WDT 1s, fACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                                 // to activate previously configured port settings
    clock_setup();
    uart_setup();

    while(1)
    {
        __bis_SR_register(LPM3_bits | GIE);

        uart_out(&_pck, sizeof(_pck));
    }
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
    if(++wdt_cnt == INTERVAL)
    {
        if(dht11_read() && _pck.hum_int == 0 && _pck.tmp_int == 0)
        {
            SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
            // Integer part of humidity
            _pck.hum_int = 50;
            // Decimal part of humidity
            _pck.hum_decimals = 0;
            // Integer part of temperature
            _pck.tmp_int = 30;
            // Decimal part of temperature
            _pck.tmp_decimals = 0;
            SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)
        }
        wdt_cnt = 0;

        __bic_SR_register_on_exit(LPM3_bits);  // Exit LPM3
    }
}
// End of file
