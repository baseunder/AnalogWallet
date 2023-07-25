#ifndef WALLET_FUNCTIONS_H
#define WALLET_FUNCTIONS_H
#include <Arduino.h>
void setRNG();
void writePublicKey();
bool initDevice(byte passw[]);
void walletstart(byte passw[]);
bool eccTest();
void eraseDevice();
void sign(uint8_t *hash);
void restore(String fn, byte passw[]);
#endif