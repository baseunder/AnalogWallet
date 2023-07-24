#ifndef BCKP_FUNCTIONS_H
#define BCKP_FUNCTIONS_H
#include <Arduino.h>
bool initBackup(byte p1[], byte pub1[]);
bool checkBackup(byte p1[], byte pub1[]);
bool restoreBackup(byte *p1, String fn);
#endif