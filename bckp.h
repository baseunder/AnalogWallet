#ifndef BCKP_FUNCTIONS_H
#define BCKP_FUNCTIONS_H
#include <Arduino.h>
uint8_t initBackup(byte *p1, byte *pub1);
uint8_t checkBackup(byte *p1, byte *pub1);
uint8_t restoreBackup(String fn);
#endif