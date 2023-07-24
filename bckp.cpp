#include "bckp.h"
#include <SD.h>

String filename = "0000000000000000";
void setFileName(byte pub[])
{
  filename = "";
  for (int i = 0; i < 8; i++)
  {
    if (pub[i] < 0x10)
    {
      filename += '0';
    }
    filename += String(pub[i], HEX);
  }
}
bool checkCard()
{
  if (!SD.begin(4) || SD.exists(filename))
  {
    return false;
  }
  return true;
}
bool initBackup(byte p1[], byte pub1[])
{
  setFileName(pub1);
  if (!checkCard)
    return false;
  File backupFile = SD.open(filename, FILE_WRITE);
  if (backupFile)
  {
    backupFile.write(p1, 32);
    backupFile.close();
    return checkBackup(p1, pub1);
  }
  else
  {
    return false;
  }
}

bool checkBackup(byte p1[], byte pub1[])
{
  setFileName(pub1);
  if (!checkCard)
    return false;
  File backupFile = SD.open(filename, FILE_READ);
  if (backupFile)
  {
    for (int i = 0; i < 32; i++)
    {
      if (backupFile.read() != p1[i])
      {
        return false;
      }
    }
    backupFile.close();
    return true;
  }
  else
  {
    return false;
  }
}

bool restoreBackup(byte *p1, String fn)
{
  File backupFile = SD.open(fn, FILE_READ);
  if (backupFile)
  {
    for (int i = 0; i < 32; i++)
    {
      p1[i] = backupFile.read();
    }
    backupFile.close();
    return true;
  }
  else
  {
    return false;
  }
}