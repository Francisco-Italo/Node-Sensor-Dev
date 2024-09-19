#include <msp430.h> 
/**
 * Defines
 */
#define DHT11 BIT6
#define SENS_IN P1IN
#define SENS_OUT P1OUT
#define SENS_DIR P1DIR
#define SENS_INT P1IFG
#define MCLK_FREQ_MHZ 1
#define FRAM_TEST_START 0x1800
/**
 * I2C use
 */
unsigned char slaveAddress, rw;
unsigned char RX_Data[6];
unsigned char TX_Data[2];
unsigned char RX_ByteCtr;
unsigned char TX_ByteCtr;
/**
 * Variables
 */
//--------------------------------------------------------------//
// Accel
//--------------------------------------------------------------//
unsigned int xAccel;
unsigned int yAccel;
unsigned int zAccel;
//--------------------------------------------------------------//
// Gases
//--------------------------------------------------------------//
unsigned int co2Lvl;
unsigned int tvocLvl;
//--------------------------------------------------------------//
// Temp/hum
//--------------------------------------------------------------//
unsigned char hum_int,hum_decimals,tmp_int,tmp_decimals,parity;
volatile int elapsed_time,bit_cnt;
//--------------------------------------------------------------//
// FRAM
//--------------------------------------------------------------//
int *FRAM_write_ptr, data;
//--------------------------------------------------------------//
// UART
//--------------------------------------------------------------//
unsigned char buff[6];
typedef struct
{
    unsigned int tmp;
    unsigned int hum;
    unsigned int xAcc;
    unsigned int yAcc;
    unsigned int zAcc;
    unsigned int co2;
    unsigned int tvoc;
}dataPackage;
/**
 * Relevant register addresses
 */
//---------------------------------------------------------------------//
// MPU-6050
//---------------------------------------------------------------------//
const unsigned char MPU_ADDR     = 0x68;      // ADO = GND. Else, 0x69
const unsigned char PWR_MGMT_1   = 0x6B;
const unsigned char ACCEL_XOUT_H = 0x3B;
//---------------------------------------------------------------------//
// CCS811
//---------------------------------------------------------------------//
const unsigned char CCS811_ADDR            = 0x5A;
const unsigned char CCS811_MEAS_MODE       = 0x01;
const unsigned char CCS811_ALG_RESULT_DATA = 0x02;
/**
 * Prototypes
 */
//------------------------------------------------//
// Clock
//------------------------------------------------//
void clock_setup();
void Software_Trim(void);
//------------------------------------------------//
// GPIO
//------------------------------------------------//
void init_gpio_unused();
//------------------------------------------------//
// FRAM unit
//------------------------------------------------//
void FRAMWrite(int);
void write_memory(int);
//------------------------------------------------//
// UART
//------------------------------------------------//
void UARTConf(void);
void convIntToStr(unsigned int, unsigned char*);
void UARTOut(unsigned char*);
//------------------------------------------------//
// DHT11
//------------------------------------------------//
unsigned char recv_sig(void);
int dht11(void);
//------------------------------------------------//
// I2C
//------------------------------------------------//
void acc_setup(void);
void acc_comm(void);
void gas_setup(void);
void gas_comm(void);
void i2c_conf(void);
void i2c_trans(unsigned char, unsigned char, unsigned char);
/**
 * Main function
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    clock_setup();
    init_gpio_unused();

    i2c_conf();
    UARTConf();
    acc_setup();
    gas_setup();

    unsigned char i;

    while (1)
    {
        FRAM_write_ptr = (int *)FRAM_TEST_START;

        acc_comm();
        gas_comm();

        /*write_memory(xAccel);
        write_memory(yAccel);
        write_memory(zAccel);
        write_memory(co2Lvl);
        write_memory(tvocLvl);*/
        write_memory(dataPackage.xAcc);
        write_memory(dataPackage.yAcc);
        write_memory(dataPackage.zAcc);
        write_memory(dataPackage.co2);
        write_memory(dataPackage.tvoc);

        if(dht11())
        {
            /*write_memory((int)hum_int);
            write_memory((int)hum_decimals);
            write_memory((int)tmp_int);
            write_memory((int)tmp_decimals);*/
            dataPackage.hum = hum_int << 8 | hum_decimals;
            dataPackage.hum = tmp_int << 8 | tmp_decimals;
            write_memory(dataPackage.hum);
            write_memory(dataPackage.tmp);
        }

        FRAM_write_ptr = (int *)FRAM_TEST_START;
        for(i = 0; i < 9; ++i)
        {
            data = *FRAM_write_ptr++;
            convIntToStr(data, buff);
            UARTOut(buff);
        }

        //__delay_cycles(500000);
    }
}
/**
 * Clock & GPIO functions
 */
//-------------------------------------------------------------------------------------------------//
// Setting up clock signal
//-------------------------------------------------------------------------------------------------//
void clock_setup()
{
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    Software_Trim();
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source
}
//-------------------------------------------------------------------------------------------------//
// Software Trim to get the best DCOFTRIM value
//-------------------------------------------------------------------------------------------------//
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
/**
 * GPIO high-impedance cutting
 */
void init_gpio_unused()
{
    PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                             // to activate previously configured port settings
    P2DIR = 0xFF; P3DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00;
    P1IFG = 0x00; P2IFG = 0x00;
}
/**
 * FRAM memory module
 */
void FRAMWrite (int value)
{
    SYSCFG0 = FRWPPW | PFWP;
    *FRAM_write_ptr = value;
    SYSCFG0 = FRWPPW | DFWP | PFWP;
}
void write_memory(int sensor_vl)
{
    FRAMWrite(sensor_vl);
    FRAM_write_ptr++;
    __delay_cycles(250);
}
/**
 * Temperature and humidity sensor routines
 */
//-------------------------------------------------------------------------------------------------//
// DHT11 comm. signal processing
//-------------------------------------------------------------------------------------------------//
unsigned char recv_sig(void)
{
    unsigned char val = 0;

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
//-------------------------------------------------------------------------------------------------//
// DHT11 handler
//-------------------------------------------------------------------------------------------------//
int dht11(void)
{
    volatile unsigned char temp;

    __delay_cycles(1100000);        // Sensor needs time between readings

    SENS_DIR |= DHT11; // Makes the pin OUTput
    SENS_OUT &= ~DHT11;// Start condition generation for DHT11
    __delay_cycles(20000); // delay ~20 milisecond for each scan

    SENS_OUT |= DHT11;// Start condition generation for DHT11
    SENS_DIR &= ~DHT11; // Makes the pin input
    SENS_INT &= ~DHT11; // Clears pin flag, if high

    while((SENS_IN&DHT11) == DHT11); // wait till the slave pulls the pin low

    //DHT11 has responded
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
/*
 * UART related functions
 */
//-------------------------------------------------------------------------------------------------//
// UART set-up
//-------------------------------------------------------------------------------------------------//
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
//-------------------------------------------------------------------------------------------------//
// Convert numbers to string for UART transmission
//-------------------------------------------------------------------------------------------------//
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
//-------------------------------------------------------------------------------------------------//
// Output function
//-------------------------------------------------------------------------------------------------//
void UARTOut(unsigned char *str)
{
  while(*str)
  {
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = *str++;
  }
}
/**
 * Accelerometer routines
 */
//-------------------------------------------------------------------------------------------------//
// MPU setup
//-------------------------------------------------------------------------------------------------//
void acc_setup()
{
    // Wake up the MPU-6050
    slaveAddress = MPU_ADDR;
    TX_Data[1] = PWR_MGMT_1;
    TX_Data[0] = 0x08;      // Set 8 MHz clock; disable temperature sensor
    TX_ByteCtr = 2;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
    __delay_cycles(32000);                // According to datasheet, hold ~30 ms
                                            // Because of gyroscope based clock oscillator
}
//-------------------------------------------------------------------------------------------------//
// Communication with accelerometer device
//-------------------------------------------------------------------------------------------------//
void acc_comm()
{
    // I2C Transaction
    slaveAddress = MPU_ADDR;            // Register pointing
    TX_Data[0] = ACCEL_XOUT_H;          // First address of the set
    TX_ByteCtr = 1;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
    RX_ByteCtr = 6;                     // Read six bytes of data
    rw = 0;
    i2c_trans(slaveAddress, rw, RX_ByteCtr);

    xAccel  = RX_Data[5] << 8 | RX_Data[4];
    yAccel  = RX_Data[3] << 8 | RX_Data[2];
    zAccel  = RX_Data[1] << 8 | RX_Data[0];
}
/**
 * Gases sensor routines
 */
//-------------------------------------------------------------------------------------------------//
// CCS811 setup
//-------------------------------------------------------------------------------------------------//
void gas_setup()
{
    slaveAddress = CCS811_ADDR;
    TX_Data[1] = CCS811_MEAS_MODE;
    TX_Data[0] = 0x10;                  // Put CCS to normal mode, no interrupt enable
    TX_ByteCtr = 2;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
}
//-------------------------------------------------------------------------------------------------//
// Communication with gases sensor
//-------------------------------------------------------------------------------------------------//
void gas_comm()
{
    // Read data
    slaveAddress = CCS811_ADDR;
    TX_Data[0] = CCS811_ALG_RESULT_DATA;
    TX_ByteCtr = 1;
    rw = 1;
    i2c_trans(slaveAddress, rw, TX_ByteCtr);
    RX_ByteCtr = 8;                  // Reading operation of environment data register
    rw = 0;
    i2c_trans(slaveAddress, rw, RX_ByteCtr);

    co2Lvl = (RX_Data[0] << 8 | RX_Data[1]);
    tvocLvl = (RX_Data[2] << 8 | RX_Data[3]);
}
/**
 * I2C related routines
 */
//-------------------------------------------------------------------------------------------------//
// Configuration of I2C module
//-------------------------------------------------------------------------------------------------//
void i2c_conf(void)
{
    // I2C pins configuration
    P1SEL0 |= BIT2 | BIT3;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                    // Software reset enabled

    UCB0CTLW0 |= UCMODE_3 | UCMST;          // I2C mode, Master mode, SMCLK
    UCB0CTLW0 |= UCSSEL__SMCLK | UCSYNC;    // Use SMCLK as clock source, sync
    UCB0CTLW1 |= UCASTP_2;                  // Automatic STOP generated by UCB0TBCNT
    UCB0BR0 = 0x000A;                       // baudrate = SMCLK / 10 = ~100 kHz

    UCB0CTLW0 &= ~UCSWRST;                   // Disable SW reset
}
//-------------------------------------------------------------------------------------------------//
// I2C transaction function
//-------------------------------------------------------------------------------------------------//
void i2c_trans(unsigned char addr, unsigned char rw, unsigned char byteCtr)
{
    while(UCB0CTL1&UCTXSTP);

    UCB0CTLW0 |= UCSWRST;
    UCB0I2CSA = addr;                   // Slave address
    UCB0TBCNT = byteCtr;
    UCB0CTLW0 &= ~UCSWRST;
    UCB0IE |= UCTXIE0 | UCRXIE0 | UCBCNTIE;

    (rw) ? (UCB0CTL1 |= UCTR):(UCB0CTL1 &= ~UCTR);  // Clear -> receiver / Set -> transmitter
    UCB0CTL1 |= UCTXSTT;

    __bis_SR_register(LPM0_bits|GIE);   // Enter LPM0 w/ interrupt
}
//-------------------------------------------------------------------------------------------------//
// UCB0 ISR
//-------------------------------------------------------------------------------------------------//
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCIB0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
      case USCI_I2C_UCNACKIFG:
          UCB0CTL1 |= UCTXSTT;                      //resend start if NACK
          break;
      case USCI_I2C_UCRXIFG0:
          RX_Data[--RX_ByteCtr] = UCB0RXBUF;  // Get received byte
          break;
      case USCI_I2C_UCTXIFG0:
          UCB0TXBUF = TX_Data[--TX_ByteCtr];  // Passes byte to transmit buffer
          break;
      case USCI_I2C_UCBCNTIFG:
          UCB0CTL1 |= UCTXSTP;
          __bic_SR_register_on_exit(CPUOFF);  // Exit LPM0
          break;
  }
}
// End program
