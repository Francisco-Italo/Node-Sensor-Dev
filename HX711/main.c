/**
 * Excitation+ (E+)
 * Excitation- (E-)
 * Output/Signal/Amplifier+ (O/S/A+)
 * Output/Signal/Amplifier- (O/S/A-)
 *
 */

#include "msp_conf.h"
#include "uart/uart.h"
#include "hx711/hx711.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop watchdog timer

    uart_init();
    scale_init();

    char i;
    long y;
    long tare;
    float weight;
    unsigned char buffer[20]; // Buffer para armazenar a string com o valor de y

    for(i = 16; i > 0; --i)
    {
        tare += scale_read();
    }
    tare = -tare;
    tare >>= 4;

    while (1)
    {
        for(i = 16; i > 0; --i)
        {
            y += scale_read(); y += tare;
        }
        y >>= 4;

        weight = scale_get_weight(y);

        float_conv(weight, buffer);
        serial_out(buffer); // Enviar o valor de y pela UART
        serial_out("\n\n");

        y ^= y;
    }
}
