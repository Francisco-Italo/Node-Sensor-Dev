/*
 * dht11.h
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#ifndef DHT_DHT11_H_
#define DHT_DHT11_H_

enum dht11_status
{
    STATUS_OK = 0,
    STATUS_CHECKSUM_ERR = -1,
    STATUS_TIMEOUT = -2
};

signed char recv_signal(void);
int dht11_read(void);

#endif /* DHT_DHT11_H_ */
