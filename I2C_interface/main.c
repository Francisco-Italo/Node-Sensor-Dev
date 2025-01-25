#include "msp_conf.h"
#include "clk/clk.h"
#include "uart/uart.h"
#include "fram/var.h"
#include "i2c/i2c.h"
#include "i2c/sensors.h"

#define SLEEP_CONS  5                  // 5s

/*
 * WDT
 */
volatile unsigned int wdt_cnt;

/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDT_ADLY_1000;                   // WDT 1s, fACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    clk_setup();
    uart_init();

	i2c_conf();

	acc_setup();
	gas_setup();

	while(1)
	{
        __bis_SR_register(LPM3_bits|GIE);   // Enter LPM0 w/ interrupt
        uart_out(&_accel_pck, sizeof(_accel_pck));
        uart_out(&_co2_pck, sizeof(_co2_pck));
	}
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
    wdt_cnt++;
    if(wdt_cnt == SLEEP_CONS)
    {
        acc_comm();
        gas_comm();

        wdt_cnt = 0;

        __bic_SR_register_on_exit(LPM3_bits);
    }
}
// End program
