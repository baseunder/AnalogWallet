#include "bckp.h"
#include <EEPROM.h>
#include "SDCARDmodded.h"
//#include "RawSD.h"
#include <SPI.h>

uint8_t statb;
int port = 4;
unsigned long block = 1000;

unsigned char buffer[512];
unsigned char error;
bool performInit = true;
uint8_t checkCard()
{
  if (performInit){
    performInit = false;
    pinMode(port, OUTPUT);
    SPI.begin();
  }
  error = SDCARDmodded.readblock(block, port);
  if (error)
  {
    return error;
  }
  return 0;
}

uint8_t initBackup(byte *p1, byte *pub1)
{
  statb = checkCard();
  if (statb) return statb;
  memcpy(buffer, p1, 32);
  for (int i = 0; i < 16; i++)
  {
    error = SDCARDmodded.writeblock(block*i, port);
    if (error!=0)return 9;
  }
  for (int i = 64+pub1[0]; i < 64+pub1[0]+16; i++)
  {
    error = SDCARDmodded.writeblock(block*i, port);
    if (error!=0)return 9;
  }
  for (int i = 400+pub1[1]; i < 400+pub1[1]+16; i++)
  {
    error = SDCARDmodded.writeblock(block*i, port);
    if (error!=0)return 9;
  }
  return 0;
}

uint8_t checkBackup(byte *p1, byte *pub1)
{
  statb = checkCard();
  if (statb) return statb;
  if (memcmp(buffer, p1, 32)==0){
    return 0;
  }else{
    return 11;
  }
}

uint8_t restoreBackup()
{
  statb = checkCard();
  if (statb) return statb;
  unsigned char *mm = (unsigned char*)buffer;
  if (memcmp(mm, mm+1, 32)==0) {
    return 13;
  }else{
    for (int i = 0; i < 32; i++)
    {
      EEPROM.update(i, buffer[i]);
    }
    return 0;
  }
}