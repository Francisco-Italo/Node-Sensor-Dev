/*
 * hx711.c
 *
 *  Created on: 3 de dez de 2024
 *      Author: italo
 */

#include "../msp_conf.h"
#include "hx711.h"

#define HX711_SCK_PORT  P1OUT
#define HX711_SCK_PIN   BIT6
#define HX711_DT_PORT   P1IN
#define HX711_DT_PIN    BIT7

#define GAIN            1          // Gain of 128 -> 1 additional clock pulse
#define CALIBRATION_FACTOR 21.44

void scale_init(void)
{
     // Configuração dos pinos do HX711
    P1DIR |= HX711_SCK_PIN; // Define SCK como saída
    P1OUT &= ~HX711_SCK_PIN; // Garante que SCK esteja em LOW

    P1DIR &= ~HX711_DT_PIN; // Define DT como entrada
    P1REN |= HX711_DT_PIN; // Habilita resistor de pull-up
    //P1OUT |= HX711_DT_PIN; // Define resistor de pull-up

    // Aguarde o HX711 estar pronto (estabilização)
    _delay_cycles(2000);
}

long scale_read(void)
{
 //  / Inicializa a variável para armazenar o valor lido
    long value = 0;
    char i = 0;

    // Aguarda o HX711 estar pronto para a leitura
    while (P1IN & HX711_DT_PIN);

    // Faz a leitura dos 24 bits do HX711
    for ( i = 0; i < 24; i++) {
        P1OUT |= HX711_SCK_PIN; // SCK em HIGH
        //__delay_cycles(10); // Pequeno atraso (ajuste conforme necessário)

        // Acumula o bit lido (os bits são lidos do MSB para o LSB)
        value = (value << 1) | ((P1IN & HX711_DT_PIN) ? (1) : (0));

        P1OUT &= ~HX711_SCK_PIN; // SCK em LOW
        _delay_cycles(3500);
    }

    // Additional pulse to represent gain
    for(i = 0; i < GAIN; ++i)
    {
        P1OUT |= HX711_SCK_PIN; // SCK em HIGH
        //__delay_cycles(10); // Pequeno atraso (ajuste conforme necessário)
        P1OUT &= ~HX711_SCK_PIN; // SCK em LOW
        _delay_cycles(3500);
    }

    if(value & 0x800000)
    {
        value |= 0xFF000000;
    }

    return value;
}

float scale_get_weight(long raw_value)
{
    return (float)raw_value / CALIBRATION_FACTOR;
}
// End of file
