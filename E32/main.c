#include <msp430.h> 
#include "clk/clk.h"
#include "uart/uart.h"

#define INTERVAL 10

void e32_sleep_mode(void);
void e32_wakeup(void);

volatile int data = 0;
volatile unsigned int wdt_cnt = 0;
volatile unsigned char buff[6];

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDT_ADLY_1000;                 // WDT 1000ms, ACLK, interval timer
    SFRIE1 |= WDTIE;                        // Enable WDT interrupt

    clk_setup();

    uart_init();

    P1DIR |= BIT0; P1OUT &= ~(BIT0);         // Debug
    P2DIR |= BIT0;                          // M0-M1
    P2DIR &= BIT1;                          // AUX

    e32_sleep_mode();

    __bis_SR_register(LPM3_bits | GIE);     // Enter LPM3
    __no_operation();                       // For debug
	return 0;
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
    wdt_cnt++;
    if(wdt_cnt == INTERVAL)
    {
        P1OUT ^= BIT0;
        e32_wakeup();
        int_conv(data,buff);
        serial_out(buff);
        //uart_out(&data,sizeof(data));
        data++;
        e32_sleep_mode();
        wdt_cnt = 0;
    }
}

void e32_sleep_mode()
{
    while(!(P2IN&BIT1));
    __delay_cycles(200000);             // Approximately 2ms
    P2OUT |= BIT0;
}

void e32_wakeup()
{
    P2OUT &= ~BIT0;
}
