/*
 * dht11.c
 *
 *  Created on: 30 de nov de 2024
 *      Author: italo
 */
#include "../msp_conf.h"
#include "../fram/var.h"
#include "intrinsics.h"
#include "dht11.h"

/**
 * Função principal para leitura do sensor DHT11 usando polling
 */
dht_status_t dht11(void) {
    char data[5] = {0};
    unsigned int pulse_width;
    char bit_cnt, byte_cnt;

    // 1. Inicia comunicação com o sensor (pino LOW por 18 ms)
    SENS_DIR |= DHT11_PIN;        // Define pino como saída
    SENS_OUT &= ~DHT11_PIN;       // Coloca em nível baixo
    __delay_cycles(18000);               // Aguarda 18ms

    // 2. Prepara para receber resposta
    SENS_OUT |= DHT11_PIN;        // Coloca nível alto
    __delay_cycles(40);                  // Aguarda 40us
    SENS_DIR &= ~DHT11_PIN;       // Define como entrada
    SENS_REN |= DHT11_PIN;        // Habilita pull-up
    SENS_OUT |= DHT11_PIN;        // Ativa pull-up

    // 3. Mede resposta LOW (~80us)
    pulse_width = RESPONSE_TIMEOUT_US;
    do{
        if (--pulse_width == 0)
            return STATUS_TIMEOUT_RESPONSE;
    }while (!(SENS_IN&DHT11_PIN));

    // 4. Mede resposta HIGH (~80us)
    pulse_width = RESPONSE_TIMEOUT_US;
    do{
        if (--pulse_width == 0)
            return STATUS_TIMEOUT_RESPONSE;
    }while ((SENS_IN&DHT11_PIN));

    // 5. Leitura de 40 bits (5 bytes)
    for (byte_cnt = 0; byte_cnt < 5; ++byte_cnt) {
        for (bit_cnt = 0; bit_cnt < 8; ++bit_cnt) {

            // Aguarda início do pulso HIGH
            pulse_width = BIT_TIMEOUT_US;
            do{
                if (--pulse_width == 0)
                    return STATUS_TIMEOUT_BIT;
            }while (!(SENS_IN&DHT11_PIN));

            // Mede duração do pulso HIGH
            unsigned int start_time = TA0R;
            while (SENS_IN & DHT11_PIN) {
                if ((TA0R - start_time) > BIT_TIMEOUT_US)
                    return STATUS_TIMEOUT_BIT;
            }
            pulse_width = TA0R - start_time;

            // Interpreta bit com base na largura do pulso
            if (pulse_width > ZERO_THRESHOLD_US) {
                data[byte_cnt] |= (1 << (7 - bit_cnt));
            }
        }
    }

    // 6. Verifica checksum
    if (data[4] != (char)(data[0] + data[1] + data[2] + data[3])) {
        return STATUS_CHECKSUM_ERROR;
    }

    // 7. Armazena dados em FRAM
    SYSCFG0 = FRWPPW | DFWP; // Libera escrita
    _pck.hum_int = data[0];
    _pck.hum_decimals = data[1];
    _pck.tmp_int = data[2];
    _pck.tmp_decimals = data[3];
    _pck.checksum = data[4];
    SYSCFG0 = FRWPPW | PFWP | DFWP; // Protege FRAM

    return STATUS_OK;
}
// End of file