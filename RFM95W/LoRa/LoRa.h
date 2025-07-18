#ifndef LORA_H
#define LORA_H

void begin(long frequency);
void send(const char* data, char len);
void sleep();
void writeRegister(char reg, char value);
char readRegister(char reg);
void setTxPower(char level);
void setSignalBandwidth(long bw);
void setSpreadingFactor(char sf);
void setCodingRate4(char denominator);
void setPreambleLength(char length);
void setSyncWord(char sw);

#endif
// End LORA_H
