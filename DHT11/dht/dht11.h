/*
 * dht11.h
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */

#ifndef DHT_DHT11_H_
#define DHT_DHT11_H_


#define DHT11_PIN BIT0
#define SENS_DIR    P1DIR // Registrador de direção
#define SENS_OUT    P1OUT // Registrador de saída
#define SENS_IN     P1IN  // Registrador de entrada
#define SENS_REN    P1REN // Registrador de resistor

#define START_DELAY_US          18000  // 18 ms
#define RESPONSE_TIMEOUT_US     100    // 100 us (timeout para resposta)
#define BIT_TIMEOUT_US          100    // 100 us (timeout para bits)
#define ZERO_THRESHOLD_US       40     // Limite para bit '0' (us)

typedef enum
{
    STATUS_OK = 0,
    STATUS_CHECKSUM_ERROR = -1,
    STATUS_TIMEOUT_BIT = -2,
    STATUS_TIMEOUT_RESPONSE = -3
}dht_status_t;

dht_status_t dht11(void);

#endif /* DHT_DHT11_H_ */
