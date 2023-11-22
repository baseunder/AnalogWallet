#include "bckp.h"
#include <EEPROM.h>
#include "RawSD.h"

uint8_t statb;

unsigned long block = 1000;
unsigned char buffer[512];
bool performInit = true;
RawSD mySD(10);
uint8_t checkCard()
{
  if (performInit){
    performInit = false;
    if (!mySD.begin()){
      return 7;
    }
  }
  if (mySD.readBlock(block, buffer))
  {
    return 0;
  }else{
    return 7;
  }
}

uint8_t initBackup(byte *p1, byte *pub1)
{
  statb = checkCard();
  if (statb) return statb;
  memcpy(buffer, p1, 32);
  for (int i = 0; i < 16; i++)
  {
    if (!mySD.writeBlock(block*i, buffer))return 9;
  }
  /*
  for (int i = 64+pub1[0]; i < 64+pub1[0]+16; i++)
  {
    if (!mySD.writeBlock(block*i, buffer))return 9;
  }
  for (int i = 400+pub1[1]; i < 400+pub1[1]+16; i++)
  {
    if (!mySD.writeBlock(block*i, buffer))return 9;
  }*/
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
  if (memcmp(mm, mm+1, 31)==0) {
    return 13;
  }else{
    for (int i = 0; i < 32; i++)
    {
      EEPROM.update(i, buffer[i]);
    }
    return 0;
  }
}