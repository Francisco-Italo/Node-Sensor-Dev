/*
 * uart.h
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#ifndef UART_UART_H_
#define UART_UART_H_

void UARTConf(void);
void UARTOut(unsigned char*);
void convIntToStr(unsigned int, unsigned char*);

#endif /* UART_UART_H_ */