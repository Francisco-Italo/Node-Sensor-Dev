#include <msp430.h> 

#define DHT11 BIT6
#define SENS_IN P1IN
#define SENS_OUT P1OUT
#define SENS_DIR P1DIR
#define SENS_INT P1IFG

#define FRAM_TEST_START 0x1800
#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

volatile int elapsed_time,bit_cnt;
unsigned int *FRAM_write_ptr, data;
unsigned char buff[6];
unsigned char hum_int,hum_decimals,tmp_int,tmp_decimals,parity;

void Software_Trim(void);                       // Software Trim to get the best DCOFTRIM value
void UARTConf(void);
void UARTOut(unsigned char*);
void convIntToStr(unsigned int, unsigned char*);
void FRAMWrite(int);

void Software_Trim(void)
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

void clock_setup()
{
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    Software_Trim();                        // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source
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

void FRAMWrite (int value)
{
    SYSCFG0 = FRWPPW | PFWP;
    *FRAM_write_ptr = value;
    SYSCFG0 = FRWPPW | DFWP | PFWP;
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

/**
 * DHT11 comm. signal processing
 */
unsigned char recv_sig(void)
{
    char val = 0;

    for(bit_cnt=0;bit_cnt<8;bit_cnt++) // for humidity first byte
    {
        while(!(SENS_IN&DHT11)); // skip the lower 50 uS part

        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
        }
        while((SENS_IN&DHT11)==DHT11);
        if(elapsed_time > 5)
        {
            val |= 0x01;
        }

        val <<= 1;
    }

    return (val>>=1);   // Return corrected value
}

/**
 * DHT11 handler
 */
int dht11(void)
{
    __delay_cycles(1100000);        // Sensor needs time between readings

    SENS_DIR |= DHT11; // Makes the pin OUTput
    SENS_OUT &= ~DHT11;// Start condition generation for DHT11
    __delay_cycles(20000); // delay ~20 milisecond for each scan

    SENS_OUT |= DHT11;// Start condition generation for DHT11
    SENS_DIR &= ~DHT11; // Makes the pin input
    SENS_INT &= ~DHT11; // Clears pin flag, if high

    while((SENS_IN&DHT11) == DHT11); // wait till the slave pulls the pin low

    /**
     * DHT11 has responded
     *
    */
    do
    {
        elapsed_time++; // check if the elapsed time = 80 uS
    }
    while(!(SENS_IN&DHT11));

    if(elapsed_time <= 10)
    {
        elapsed_time = 0;
        do
        {
            elapsed_time++; // check if the elapsed time = 80 uS
        }
        while((SENS_IN&DHT11)==DHT11);

        if(elapsed_time <=10)// check if the elapsed time = 80 uS
        {
            // Integer part of humidity
            hum_int = recv_sig();
            // Decimal part of humidity
            hum_decimals = recv_sig();
            // Integer part of temperature
            tmp_int = recv_sig();
            // Decimal part of temperature
            tmp_decimals = recv_sig();
            // Parity checksum
            parity = recv_sig();
        }
    }

    return (((hum_int + hum_decimals + tmp_int + tmp_decimals) == parity) ? 1 : 0);
}

/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    clock_setup();
    UARTConf();
    init_gpio_unused();
    unsigned char i;

    while (1)
    {
        FRAM_write_ptr = (unsigned char *)FRAM_TEST_START;
        if(dht11())
        {
            for(i = 0; i < 4; ++i)
            {
                FRAMWrite((int)hum_int);
                FRAM_write_ptr++;
                __delay_cycles(250);
                FRAMWrite((int)hum_decimals);
                FRAM_write_ptr++;
                __delay_cycles(250);
                FRAMWrite((int)tmp_int);
                FRAM_write_ptr++;
                __delay_cycles(250);
                FRAMWrite((int)tmp_decimals);
                __delay_cycles(250);
            }
            FRAM_write_ptr = (unsigned char *)FRAM_TEST_START;
            for(i = 0; i < 4; ++i)
            {
                data = *FRAM_write_ptr++;
                convIntToStr(data, buff);
                UARTOut(buff);
                UARTOut(((i & 1) ? ("\n") : (".")));

            }
        }
    }
}
// End of file
