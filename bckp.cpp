#include "bckp.h"
#include <SD.h>
#include <EEPROM.h>

uint8_t filename[9];
uint8_t statb;
void setFileName(byte *pub)
{
  snprintf(filename, sizeof(filename), "%02X%02X%02X%02X\0", pub[0], pub[1], pub[2], pub[3]);
}
uint8_t checkCard()
{
  if (!SD.begin())
  {
    return 7;
  }
  return 0;
}
uint8_t initBackup(byte *p1, byte *pub1)
{
  setFileName(pub1);
  statb = checkCard();
  if (statb) return statb;
  File backupFile = SD.open(filename, FILE_WRITE);
  if (backupFile)
  {
    backupFile.write(p1, 32);
    backupFile.close();
    Serial.write((uint8_t)14);
    Serial.write(filename, 8);
    return 0;
  }
  else
  {
    return 9;
  }
}

uint8_t checkBackup(byte *p1, byte *pub1)
{
  setFileName(pub1);
  statb = checkCard();
  if (statb) return statb;
  File backupFile = SD.open(filename, FILE_READ);
  if (backupFile)
  {
    uint8_t tmpP1[32];
    backupFile.readBytes(tmpP1, 32);
    backupFile.close();
    if (memcmp(tmpP1, p1, 32)==0){
      return 0;
    }else{
      return 11;
    }
  }
  else
  {
    return 10;
  }
}

uint8_t restoreBackup(String fn)
{
  statb = checkCard();
  if (statb) return statb;
  if (!SD.exists(fn)) {
    return 13;
  }else{
    File backupFile = SD.open(fn, FILE_READ);
    if (backupFile)
    {
      uint8_t tmpP1[32];
      backupFile.readBytes(tmpP1,32);
      for (int i = 0; i < 32; i++)
      {
        EEPROM.update(i, tmpP1[i]);
      }
      backupFile.close();
      return 0;
    }
    else
    {
      return 10;
    }
  }
}