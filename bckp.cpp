#include "bckp.h"
#include <EEPROM.h>
#include <SD.h>

uint8_t statb;
unsigned char data[512];

uint8_t readSD(uint8_t *data, int blockNumber, int pin=4){
  Sd2Card card;
  if (card.init(SPI_QUARTER_SPEED, pin)){
    card.readBlock(blockNumber, data);
    if (memcmp(data+1, data, 511)!=0){
      return 0;
    }else{
      return 13;
    }    
  }else{
    return 7;
  }  
}
uint8_t writeAndValidSD(uint8_t *data, int blockNumber, int pin=4){
  Sd2Card card;
  if (card.init(SPI_QUARTER_SPEED, pin)){
    card.writeBlock(blockNumber, data);
    unsigned char buffer[512];
    card.readBlock(blockNumber, buffer);
    if (memcmp(buffer, data, 512)==0){
      return 0;
    }else{
      return 9;
    }    
  }else{
    return 7;
  }  
}
uint8_t secureWriteAndValidSD(uint8_t *data, int pin=4){
  if (statb=writeAndValidSD(data, 100, pin))return statb;
  if (statb=writeAndValidSD(data, 1000, pin))return statb;
  if (statb=writeAndValidSD(data, 10000, pin))return statb;
  int blockNumber = 10000;
  for (int j = 0; j < 32; j++){
    blockNumber += data[j];
    if (statb=writeAndValidSD(data, blockNumber, pin))return statb;
  }
  return statb;
}

uint8_t checkCard(int pin = 4){
  Sd2Card card;
  if (card.init(SPI_QUARTER_SPEED, pin)){
    return 0;
  }else{
    return 7;
  }
}

uint8_t initBackup(byte *p1, byte *pub1)
{
  memcpy(data, p1, 32);  
  statb = secureWriteAndValidSD(data);
  memcpy(data, {0}, 32);
  return statb;
}

uint8_t checkBackup(byte *p1, byte *pub1)
{
  statb = readSD(data, 10000);
  if (statb) return statb;
  if (memcmp(data, p1, 32)){
    return 11;
  }else{
    return 0;
  }
}

uint8_t restoreBackup()
{
  statb = readSD(data, 10000);
  if (statb) return statb;
  if (memcmp(data, data+1, 31)) {
    for (int i = 0; i < 32; i++)
    {
      EEPROM.update(i, data[i]);
    }
    return 0;
  }else{
    return 13;
  }
}