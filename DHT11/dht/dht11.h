/*
 * dht11.h
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#ifndef DHT_DHT11_H_
#define DHT_DHT11_H_

extern unsigned char hum_int,hum_decimals,tmp_int,tmp_decimals, parity;

unsigned char recv_sig(void);
int dht11(void);

#endif /* DHT_DHT11_H_ */
