#include <msp430.h>
#include "SPI.h"

#define NSS_PIN     BIT0
#define NSS_PORT    P1OUT
#define NSS_DIR     P1DIR

void spi_config() {
    
}

void init_GPIO() {
    NSS_DIR |= NSS_PIN;
    NSS_PORT |= NSS_PIN;  // CS high

    P1SEL0 |= BIT4 | BIT5 | BIT6; // P1.4 (MOSI), P1.5 (MISO), P1.6 (SCK)
    P1SEL1 &= ~(BIT4 | BIT5 | BIT6);
}

void init() {
    UCA0CTLW0 = UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK | UCMST | UCSYNC | UCMSB | UCCKPH;
    UCA0BRW = 2;
    UCA0CTLW0 &= ~UCSWRST;
}

void cs_low() {
    NSS_PORT &= ~NSS_PIN;
}

void cs_high() {
    NSS_PORT |= NSS_PIN;
}

char transfer(char data) {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = data;
    while (!(UCA0IFG & UCRXIFG));
    return UCA0RXBUF;
}
// End of file
