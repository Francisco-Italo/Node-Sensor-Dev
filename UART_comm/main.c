//******************************************************************************
//   MSP430FR24xx Demo - USCI_A0 External Loopback test @ 9600 baud
//
//  Description: This demo connects TX to RX of the MSP430 UART
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data. If data is incorrect P1.0 LED is
//  turned ON.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = DCODIV ~1MHz.
//
//                MSP430FR2433
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |                 |
//            |     P1.4/UCA0TXD|----
//            |                 |   |
//            |     P1.5/UCA0RXD|----
//            |                 |
//            |            P1.0 |--> LED
//            |                 |
//             -----------------
//******************************************************************************
#include "msp_conf.h"
#include "uart/uart.h"

unsigned char TXData[] = "Hello Embedded World\n\r";

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop watchdog timer

    // clock setup
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    Software_Trim();                        // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                              // to activate previously configured port settings
    P1DIR |= BIT0;
    P1OUT &= ~(BIT0);                           // P1.0 out low

    // Configure pins to UART mode
    P1SEL0 |= BIT4 | BIT5;                    // set 2-UART pin as second function

    // Configure UART
    UCA0CTLW0 |= UCSWRST;                     // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;

    // Baud Rate calculation (infos in UG table 22-5)
    UCA0BR0 = 6;                              // 1000000/16/9600 = 6.51
    UCA0BR1 = 0;
    UCA0MCTLW = 0x2000 | UCOS16 | UCBRF_8;      // 1000000/16/9600 - INT(1000000/16/9600)=0.51
                                                // UCBRFx = int (0.51*16) = 8; 16 bcz of UCOS16
                                                // UCBRSx value = 0x20
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    while (1)
    {
        P1OUT ^= BIT0;
        ser_output(TXData);
        __delay_cycles(1000000);
    }
}
// End of file
