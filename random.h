#ifndef RANDOM_FUNCTIONS_H
#define RANDOM_FUNCTIONS_H
#include <Arduino.h>
int getRead();
void flickrTest();
void updateSHA(byte entro);
uint8_t getTrueRotateRandomByteWithSHAupdate();
uint8_t getTrueRotateRandomByte();
int RNG(uint8_t *dest, unsigned size);
void getshavalue(uint8_t *dest);
#endif