/*
 * hx711.h
 *
 *  Created on: 3 de dez de 2024
 *      Author: italo
 */

#ifndef UART_UART_H_
#define UART_UART_H_

void uart_init(void);
void Software_Trim(void);                       // Software Trim to get the best DCOFTRIM value
void serial_out(unsigned char*);
void int_conv(unsigned int, unsigned char*);
void float_conv(float, unsigned char*);

#endif /* UART_UART_H_ */