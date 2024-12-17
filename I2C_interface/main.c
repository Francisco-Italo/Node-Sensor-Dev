#include "msp_conf.h"
#include "i2c/i2c.h"
#include "i2c/sensors.h"

#define SLEEP_CONS  9375                  // 5 mins.

/*
 * WDT
 */
volatile unsigned int wdt_cnt;

/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDT_MDLY_32;                   // WDT 32ms, SMCLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

	i2c_conf();

	//acc_setup();
	gas_setup();

    __bis_SR_register(LPM0_bits|GIE);   // Enter LPM0 w/ interrupt
}

//-------------------------------------------------------------------------------------------------//
// WDT ISR
//-------------------------------------------------------------------------------------------------//
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) WDT_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if(++wdt_cnt == SLEEP_CONS)
    {
        //acc_comm();
        gas_comm();

        wdt_cnt = 0;
    }
    __bic_SR_register_on_exit(CPUOFF);  // Exit LPM0
}
// End program
