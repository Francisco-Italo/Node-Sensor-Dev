//******************************************************************************
//   MSP430FR235x Demo - Long word writes to FRAM
//
//   Description: Use long word write to write to 512 byte blocks of info FRAM.
//   Toggle LED after every 100 writes.
//   ACLK = REFO, MCLK = SMCLK = default DCODIV = ~1MHz
//
//           MSP430FR2355
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |          P1.0 |---> LED
//
//   Darren Lu
//   Texas Instruments Inc.
//   Oct. 2016
//   Built with IAR Embedded Workbench v6.50 & Code Composer Studio v6.2
//******************************************************************************
#include "msp_conf.h"
#include "clk/clk.h"
#include "uart/uart.h"
#include "fram/fram.h"

#define FRAM_TEST_START 0x1800

unsigned int *FRAM_write_ptr, data;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output directionOUT

    UARTConf();

    data = 0x0001;                      // Initialize dummy data
    unsigned char buff[6];
    unsigned char count = 0;

    while(1)
    {
        data += 0x0001;
        FRAM_write_ptr = (unsigned int *)FRAM_TEST_START;
        FRAMWrite();
        count++;
        if (count > 4)
        {
            FRAM_write_ptr = (unsigned int *)FRAM_TEST_START;

            unsigned char i;
            for(i = 0; i < 5; ++i)
            {
                data = *FRAM_write_ptr++;
                convIntToStr(data, buff);
                UARTOut(buff);
            }

            P1OUT ^= 0x01;                  // Toggle LED to show 512 bytes
            count = 0;                      // have been written
            data = 0x0001;
        }
    }
}
// End of file
