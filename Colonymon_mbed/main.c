#include <msp430.h>
#include "clk/clk.h"
#include "fram/fram.h"
#include "uart/uart.h"
#include "dht/dht11.h"
#include "hx711/hx711.h"
#include "i2c/i2c.h"
#include "i2c/sensors.h"

/**
 * Sleep constant for cpu waking up
 */
#define SLEEP_CONS  3                  // 5 mins. -> 300
/**
 * Global variables
 */
volatile int wdt_cnt;                  // WDT
/**
 * Prototypes
 */
void init_gpio_unused();
/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDT_ADLY_1000;                 // WDT 1000ms, ACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    clock_setup();
    init_gpio_unused();

    i2c_init();
    uart_init();
    //acc_setup();
    gas_setup();
    scale_init();

    while(1)
    {
        __bis_SR_register(LPM3_bits | GIE);
        
        char i;
        unsigned long w = 0, t;

        //acc_comm();
        gas_comm();

        SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
        t = _pck.sensor_data._weight_pck.tare;
        SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)
        for(i = 16; i > 0; --i)
        {
            w += scale_read();
            w -= t;
        }
        w >>= 4;
        if(w > 0xFF00 && _pck.sensor_data._weight_pck.raw_weight < 0xFF00)
        {
            w &= 0x000000FF;
        }
        SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
        _pck.sensor_data._weight_pck.raw_weight = w;
        SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)

        while(dht11() != STATUS_OK);

        uart_out(_pck.tx_block, sizeof(_pck.tx_block)); // Transmission to gateway
    }
}
/**
 * WDT ISR
 */
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
        wdt_cnt = 0;
        __bic_SR_register_on_exit(LPM3_bits);  // Exit LPM
    }
}
/**
 * GPIO high-impedance cutting
 */
void init_gpio_unused()
{
    PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                             // to activate previously configured port settings
    /*P2DIR = 0xFF; P3DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00;
    P1IFG = 0x00; P2IFG = 0x00;*/
}
// End of file
