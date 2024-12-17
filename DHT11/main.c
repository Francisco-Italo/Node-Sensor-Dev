#include "msp_conf.h"
#include "fram/fram.h"
#include "uart/uart.h"
#include "dht/dht11.h"
#include "clk/clk.h"

#define DHT_INTERVAL  100                  // 5 mins -> 9375
#define FRAM_TEST_START 0x1800

volatile unsigned int wdt_cnt;
volatile unsigned char read_ok;

void init_gpio_unused(void);

/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDT_MDLY_32;                   // WDT 32ms, SMCLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    clock_setup();
    UARTConf();
    init_gpio_unused();

    int data;
    unsigned char buff[6], i;

    while(1)
    {
        __bis_SR_register(LPM0_bits | GIE);

        if(read_ok)
        {
            FRAM_write_ptr = (int*)FRAM_TEST_START;
            FRAMWrite(hum_decimals << 8 | hum_int);
            FRAM_write_ptr++;
            __delay_cycles(250);
            FRAMWrite(tmp_decimals << 8 | tmp_int);
            FRAM_write_ptr++;
            __delay_cycles(250);
        }
        FRAM_write_ptr = (int*)FRAM_TEST_START;
        for(i = 2; i > 0; --i)
        {
            data = *FRAM_write_ptr++;
            convIntToStr((data&0xFF), buff);
            UARTOut(buff); UARTOut((unsigned char*)".");
            convIntToStr((data>>8), buff);
            UARTOut(buff); UARTOut((unsigned char*)"\n");
        }
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
    if(++wdt_cnt == DHT_INTERVAL)
    {
        read_ok = (dht11() ? 1 : 0);
        wdt_cnt = 0;

        __bic_SR_register_on_exit(CPUOFF);  // Exit LPM0
    }
}

void init_gpio_unused()
{
    PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                             // to activate previously configured port settings
    P2DIR = 0xFF; P3DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0xFF;
    P1IFG = 0x00; P2IFG = 0x00;
}
// End of file
