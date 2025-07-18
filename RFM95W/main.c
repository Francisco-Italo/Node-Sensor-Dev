#include <msp430.h>
#include "SPI/SPI.h"
#include "LoRa/LoRa.h"

#define CONFIG_RADIO_OUTPUT_POWER   14
#define CONFIG_RADIO_BW             125000

void delay_seconds(char seconds);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Para o watchdog

    // Configurações iniciais
    init_GPIO();
    init();

    __delay_cycles(100000); // tempo para estabilização

    // Inicializa o LoRa com frequência de 915 MHz
    begin(915E6); // 915000000 Hz
    setTxPower(CONFIG_RADIO_OUTPUT_POWER);
    setSignalBandwidth(CONFIG_RADIO_BW);
    setSpreadingFactor(7);
    setCodingRate4(5);
    setPreambleLength(10);
    setSyncWord(0xAB);

    while (1) {
        send("Hello", 5);
        sleep();                    // Modo sleep do LoRa
       // __bis_SR_register(LPM3_bits + GIE); // Entra em LPM3 (opcional, pode tirar)
        delay_seconds(3);          // Espera 3 segundos (sem interrupção)
    }
}

// Função de atraso baseada em SMCLK de 1 MHz
void delay_seconds(char seconds) {
    while (seconds--) {
        __delay_cycles(1000000); // 1 segundo
    }
}
// End of file
