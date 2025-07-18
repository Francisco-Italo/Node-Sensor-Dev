#ifndef SPI_H
#define SPI_H

void spi_config(void);
void cs_low(void);
void cs_high(void);
char transfer(char data);

#endif
// End SPI_H
