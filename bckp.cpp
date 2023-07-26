#include "bckp.h"
#include <SD.h>
#include <EEPROM.h>

char filename[9];
uint8_t statb;
void setFileName(byte pub[])
{
  snprintf(filename, sizeof(filename), "%02X%02X%02X%02X", pub[0], pub[1], pub[2], pub[3]);
}
uint8_t checkCard()
{
  if (!SD.begin())
  {
    return 50;
  }
  return 0;
}
uint8_t initBackup(byte p1[], byte pub1[])
{
  setFileName(pub1);
  statb = checkCard();
  if (statb) return statb;
  File backupFile = SD.open(filename, FILE_WRITE);
  if (backupFile)
  {
    backupFile.write(p1, 32);
    backupFile.close();
    SD.end();
    return checkBackup(p1, pub1);
  }
  else
  {
    SD.end();
    return 61;
  }
}

uint8_t checkBackup(byte p1[], byte pub1[])
{
  setFileName(pub1);
  if (checkCard())
    return 70;
  File backupFile = SD.open(filename, FILE_READ);
  if (backupFile)
  {
    for (int i = 0; i < 32; i++)
    {
      if (backupFile.read() != p1[i])
      {
        SD.end();
        return 71;
      }
    }
    backupFile.close();
    SD.end();
    return 0;
  }
  else
  {
    SD.end();
    return 72;
  }
}

uint8_t restoreBackup(String fn)
{
  statb = checkCard();
  if (statb) return statb;
  File backupFile = SD.open(fn, FILE_READ);
  if (backupFile)
  {
    for (int i = 0; i < 32; i++)
    {
      EEPROM.update(i, backupFile.read());
    }
    backupFile.close();
    SD.end();
    return 0;
  }
  else
  {
    SD.end();
    return 80;
  }
}