/*
 * hx711.h
 *
 *  Created on: 3 de dez de 2024
 *      Author: italo
 */

#ifndef UART_UART_H_
#define UART_UART_H_

void uart_init(void);
void uart_out(const void*, unsigned char);
/*void int_conv(unsigned int, unsigned char*);
void float_conv(float, volatile unsigned char*);*/

#endif /* UART_UART_H_ */
