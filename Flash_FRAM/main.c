#include "msp_conf.h"
#include "clk/clk.h"
#include "uart/uart.h"

// Statically-initialized variable
#ifdef __TI_COMPILER_VERSION__
#pragma PERSISTENT(data)
int data = 0x0100;
#elif __IAR_SYSTEMS_ICC__
__persistent int data = 0x0100;
#else
// Port the following variable to an equivalent persistent functionality for the specific compiler being used
int data = 0x0100;
#endif

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output direction OUT

    uart_setup();

    while(1)
    {
        SYSCFG0 = FRWPPW | DFWP;            // Program FRAM write enable
        data += 0x0001;
        SYSCFG0 = FRWPPW | PFWP | DFWP;     // Program FRAM write protected (not writable)

        uart_out(&data, sizeof(data));
        P1OUT ^= 0x01;
        _delay_cycles(500000);
    }
}
// End of file
