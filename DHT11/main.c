#include "intrinsics.h"
#include "msp430fr2433.h"
#include "msp_conf.h"
#include "fram/var.h"
#include "uart/uart.h"
#include "dht/dht11.h"
#include "clk/clk.h"

#define INTERVAL  3                         // 5 mins -> 300
volatile unsigned int wdt_cnt;              // WDT interrupt time counter

void timer_setup(void) {
    TA0CTL = TACLR; // Limpa o registrador TAR
    TA0CTL = TASSEL__SMCLK | MC__CONTINUOUS | ID__1; // SMCLK, modo contínuo, sem divisão
}

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
    timer_setup();

    //P2DIR |= BIT4;
    //P2OUT &= ~BIT4;

    while(1)
    {
        __bis_SR_register(LPM3_bits | GIE);

        // Power-on sensor
        //P2OUT |= BIT4;

        dht_status_t status = dht11();
        switch (status)
        {
            case STATUS_OK: uart_out(&_pck, sizeof(_pck)); break;
            case STATUS_TIMEOUT_RESPONSE:
            case STATUS_TIMEOUT_BIT:
            case STATUS_CHECKSUM_ERROR:
                __delay_cycles(1000000);
                if(dht11() != STATUS_OK)
                {
                    if(_pck.checksum == 0)
                    {
                        // Valor de referência
                        SYSCFG0 = FRWPPW | DFWP; // Libera escrita
                        _pck.hum_int = 60;
                        _pck.hum_decimals = 0;
                        _pck.tmp_int = 30;
                        _pck.tmp_decimals = 0;
                        _pck.checksum = 90;
                        SYSCFG0 = FRWPPW | PFWP | DFWP; // Protege FRAM
                    }
                }
                uart_out(&_pck, sizeof(_pck));
                break;
        }
        //P2OUT &= ~BIT4;
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
    wdt_cnt++;
    if(wdt_cnt == INTERVAL)
    {
        wdt_cnt = 0;
        __bic_SR_register_on_exit(LPM3_bits);  // Exit LPM3
    }
}
// End of file
