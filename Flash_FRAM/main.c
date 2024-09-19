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
#include <msp430.h>

#define FRAM_TEST_START 0x1800
#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

void Software_Trim();                       // Software Trim to get the best DCOFTRIM value

void UARTConf(void);
void UARTOut(unsigned char*);
void convIntToStr(unsigned int, unsigned char*);
void FRAMWrite(void);

unsigned char buff[6];
unsigned int data;
unsigned int *FRAM_write_ptr;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Clock set-up
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    Software_Trim();                        // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output directionOUT

    UARTConf();

    unsigned char count = 0;
    data = 0x0001;                      // Initialize dummy data

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

void UARTConf(void)
{
    // Configure UART pins
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
}

void FRAMWrite (void)
{
    unsigned char i;
    SYSCFG0 = FRWPPW | PFWP;
    for (i = 0; i < 128; i++)
    {
        *FRAM_write_ptr++ = data;
    }
    SYSCFG0 = FRWPPW | DFWP | PFWP;
}

void convIntToStr(unsigned int num, unsigned char *str)
{
    unsigned char i = 0, j = 0;
    unsigned char aux[] = {'0','0','0','0','0','\0'};

    do
    {
      aux[i++] = num % 10 + '0';
      num /= 10;
    }
    while(num > 0);

    while(i--)
    {
      str[j++] = aux[i];
    }
    str[j] = '\0';
}

void UARTOut(unsigned char *str)
{
  while(*str)
  {
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = *str++;
  }
}

void Software_Trim()
{
    unsigned int oldDcoTap = 0xffff;
    unsigned int newDcoTap = 0xffff;
    unsigned int newDcoDelta = 0xffff;
    unsigned int bestDcoDelta = 0xffff;
    unsigned int csCtl0Copy = 0;
    unsigned int csCtl1Copy = 0;
    unsigned int csCtl0Read = 0;
    unsigned int csCtl1Read = 0;
    unsigned int dcoFreqTrim = 3;
    unsigned char endLoop = 0;

    do
    {
        CSCTL0 = 0x100;                         // DCO Tap = 256
        do
        {
            CSCTL7 &= ~DCOFFG;                  // Clear DCO fault flag
        }while (CSCTL7 & DCOFFG);               // Test DCO fault flag

        __delay_cycles((unsigned int)3000 * MCLK_FREQ_MHZ);// Wait FLL lock status (FLLUNLOCK) to be stable
                                                           // Suggest to wait 24 cycles of divided FLL reference clock
        while((CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)) && ((CSCTL7 & DCOFFG) == 0));

        csCtl0Read = CSCTL0;                   // Read CSCTL0
        csCtl1Read = CSCTL1;                   // Read CSCTL1

        oldDcoTap = newDcoTap;                 // Record DCOTAP value of last time
        newDcoTap = csCtl0Read & 0x01ff;       // Get DCOTAP value of this time
        dcoFreqTrim = (csCtl1Read & 0x0070)>>4;// Get DCOFTRIM value

        if(newDcoTap < 256)                    // DCOTAP < 256
        {
            newDcoDelta = 256 - newDcoTap;     // Delta value between DCPTAP and 256
            if((oldDcoTap != 0xffff) && (oldDcoTap >= 256)) // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim--;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM0)) | (dcoFreqTrim<<4);
            }
        }
        else                                   // DCOTAP >= 256
        {
            newDcoDelta = newDcoTap - 256;     // Delta value between DCPTAP and 256
            if(oldDcoTap < 256)                // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim++;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM0)) | (dcoFreqTrim<<4);
            }
        }

        if(newDcoDelta < bestDcoDelta)         // Record DCOTAP closest to 256
        {
            csCtl0Copy = csCtl0Read;
            csCtl1Copy = csCtl1Read;
            bestDcoDelta = newDcoDelta;
        }

    }while(endLoop == 0);                      // Poll until endLoop == 1

    CSCTL0 = csCtl0Copy;                       // Reload locked DCOTAP
    CSCTL1 = csCtl1Copy;                       // Reload locked DCOFTRIM
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
}
// End of file
