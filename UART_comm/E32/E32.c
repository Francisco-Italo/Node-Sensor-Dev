#include "intrinsics.h"
#include <msp430.h>
#include "E32.h"

void e32_set_mode(enum module_mode md)
{
    switch(md)
    {
        case NORMAL:
            P2OUT &= ~(BIT0 | BIT1);
        break;
        case WAKEUP:
            P2OUT &= ~(BIT1);
            P2OUT |= BIT0;
        break;
        case POWERDOWN:
            P2OUT &= ~(BIT0);
            P2OUT |= BIT1;
        break;
        case PROGRAM:
            P2OUT |= (BIT0 | BIT1);
        break;
    }
    //__delay_cycles(3000);   // 2 ms time to activate pins
    while(!(P2IN&BIT2));       // Wait AUX back to high
}
// End of file
