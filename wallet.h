#ifndef WALLET_FUNCTIONS_H
#define WALLET_FUNCTIONS_H
#include <Arduino.h>
void setRNG();
void writePublicKey();
uint8_t initDevice(byte passw[]);
uint8_t walletstart(byte *passw);
uint8_t eccTest();
void eraseDevice();
uint8_t sign(uint8_t *hash);
uint8_t restore(String fn, byte passw[]);
#endif