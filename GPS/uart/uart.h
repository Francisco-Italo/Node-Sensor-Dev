/*
 * uart.h
 *
 *  Created on: 10 de dez de 2024
 *      Author: italo
 */

#ifndef UART_UART_H_
#define UART_UART_H_

#define BUFFER_SIZE 100

void uart_setup(void);
void ser_output(volatile unsigned char *str);

#endif /* UART_UART_H_ */
