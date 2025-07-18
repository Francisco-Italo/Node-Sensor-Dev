#include "LoRa.h"
#include "SPI/SPI.h"
#include "msp430.h"

void writeRegister(char reg, char value) {
    cs_low();
    transfer(reg | 0x80);
    transfer(value);
    cs_high();
}

char readRegister(char reg) {
    char value;
    cs_low();
    transfer(reg & 0x7F);
    value = transfer(0x00);
    cs_high();
    return value;
}

void begin(long frequency) {
    writeRegister(0x01, 0x80); // modo sleep + LoRa
    writeRegister(0x01, 0x81); // modo standby
    writeRegister(0x06, (char)(frequency >> 16));
    writeRegister(0x07, (char)(frequency >> 8));
    writeRegister(0x08, (char)(frequency));
}

void send(const char* data, char len) {
    char i;

    writeRegister(0x0D, 0x00); // FIFO pointer
    for (i = 0; i < len; i++) {
        writeRegister(0x00, data[i]);
    }
    writeRegister(0x22, len);     // payload length
    writeRegister(0x01, 0x83);    // modo TX
    while ((readRegister(0x12) & 0x08) == 0); // aguarda TxDone
    writeRegister(0x12, 0x08);    // limpa TxDone
}

void sleep() {
    writeRegister(0x01, 0x80); // sleep + LoRa
}

void setTxPower(char level) {
    // PA config (RFO output)
    if (level < 2) level = 2;
    if (level > 17) level = 17;
    writeRegister(0x09, 0x80 | (level - 2));  // RegPaConfig
}

void setSignalBandwidth(long bw) {
    char bw_val;

    if (bw <= 7800)       bw_val = 0;
    else if (bw <= 10400) bw_val = 1;
    else if (bw <= 15600) bw_val = 2;
    else if (bw <= 20800) bw_val = 3;
    else if (bw <= 31250) bw_val = 4;
    else if (bw <= 41700) bw_val = 5;
    else if (bw <= 62500) bw_val = 6;
    else if (bw <= 125000)bw_val = 7;
    else if (bw <= 250000)bw_val = 8;
    else                  bw_val = 9;

    char config1 = readRegister(0x1D);
    config1 = (config1 & 0x0F) | (bw_val << 4);
    writeRegister(0x1D, config1);
}

void setSpreadingFactor(char sf) {
    if (sf < 6)  sf = 6;
    if (sf > 12) sf = 12;

    writeRegister(0x31, (sf == 6) ? 0xC5 : 0xC3); // RegDetectOptimize
    writeRegister(0x37, (sf == 6) ? 0x0C : 0x0A); // RegDetectionThreshold

    char config2 = readRegister(0x1E);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    writeRegister(0x1E, config2);
}

void setCodingRate4(char denominator) {
    if (denominator < 5) denominator = 5;
    if (denominator > 8) denominator = 8;
    char cr = denominator - 4;
    char config1 = readRegister(0x1D);
    config1 = (config1 & 0xF1) | (cr << 1);
    writeRegister(0x1D, config1);
}

void setPreambleLength(char length) {
    writeRegister(0x20, 0x00); // high byte (assumindo < 255)
    writeRegister(0x21, length); // low byte
}

void setSyncWord(char sw) {
    writeRegister(0x39, sw);
}
// End of file
